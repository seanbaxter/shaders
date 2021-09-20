#if __circle_build__ < 132
#error "Requires Circle build 132"
#endif

#include <mgpu/gl/mergesort.hxx>
#include <memory>

// CUDA Toolkit dumps a lot of garbage into the global namespace that 
// conflicts with GLSL declarations. Stop that with these symbols:
#define __DEVICE_FUNCTIONS_H__  // Prevent <device_functions.h>
#define __COMMON_FUNCTIONS_H__  // Prevent <crt/common_functions.h>

// Include necessary CUDA stuff.
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_functions.h>
#include <device_atomic_functions.h>
#include <device_atomic_functions.hpp>
#include <sm_30_intrinsics.h>
#include <sm_30_intrinsics.hpp>

// CUDA and CUDA-OpenGL interop.
#include <cuda_gl_interop.h>

// CUB radix sort.
#include <cub/device/device_radix_sort.cuh>

#define USE_IMGUI
#include "../include/appglfw.hxx"

using namespace mgpu::gl;

// Create dual-use buffers.
// These support OpenGL buffer access and CUDA pointer access.
template<typename T>
struct hybrid_buffer_t {
  typedef std::remove_extent_t<T> type_t;

  ~hybrid_buffer_t() {
    resize(0, false);
  }

  // Access the SSBO or CUDA pointer. For SSBO access, returns an iterator that
  // is parameterized on bind.
  template<int bind, bool use_cuda>
  auto bind() {
    if constexpr(use_cuda) {
      cudaGraphicsMapResources(1, &resource, 0);
      type_t* p;
      size_t num_bytes;
      cudaGraphicsResourceGetMappedPointer((void**)&p, &num_bytes, resource);
      return p;

    } else {
      return gl_buffer.template bind_ssbo<bind>();
    }
  }

  template<int bind, bool use_cuda>
  void unbind() {
    if constexpr(use_cuda) {
      cudaGraphicsUnmapResources(1, &resource, 0);

    } else {
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind, gl_buffer);
    }
  }

  size_t size() const noexcept {
    return gl_buffer.count;
  }

  void resize(int count, bool preserve = false) {
    if(count != size()) {
      // Unregister the old resource
      if(size())
        cudaGraphicsUnregisterResource(resource);

      gl_buffer.resize(count, preserve);

      // Register the new resource.
      if(count)
        cudaGraphicsGLRegisterBuffer(&resource, gl_buffer, 
          cudaGraphicsRegisterFlagsNone);
    }
  }

  void set_data_range(const type_t* data, int first, int count) {
    gl_buffer.set_data_range(data, first, count);
  }

  void clear_bytes() {
    gl_buffer.clear_bytes();
  }

  void swap(hybrid_buffer_t& rhs) {
    gl_buffer.swap(rhs.gl_buffer);
    std::swap(resource, rhs.resource);
  }

  gl_buffer_t<T> gl_buffer;
  cudaGraphicsResource_t resource;
};

////////////////////////////////////////////////////////////////////////////////
// Abstract launch mechanism for CUDA and OpenGL.

template<typename func_t>
__global__ void kernel(func_t func, int count) {
  int gid = threadIdx.x + blockIdx.x * blockDim.x;
  if(gid < count)
    func(gid);
}

template<bool use_cuda, typename func_t>
void transform(const func_t& func, int count) {
  if constexpr(use_cuda) {
    const int nt = 128;
    int num_blocks = (count + nt - 1) / nt;
    kernel<<<num_blocks, nt>>>(func, count);
  } else{
    gl_transform(func, count);   
  }
}

////////////////////////////////////////////////////////////////////////////////
// Manage the ping-pong buffers for CUB radix sort.
// This performs a key-index sort.

struct cub_radix_sort {
  cub_radix_sort();
  ~cub_radix_sort();
  void sort(uint32_t* keys, uint32_t* indices, size_t count);

  uint32_t* keys2;
  uint32_t* indices2;
  uint32_t* aux;
  size_t capacity, aux_bytes;
};

cub_radix_sort::cub_radix_sort() {
  keys2 = indices2 = aux = nullptr;
  capacity = 0;
}

cub_radix_sort::~cub_radix_sort() {
  if(capacity) {
    cudaFree(keys2);
    cudaFree(indices2);
    cudaFree(aux);
  }
}

void cub_radix_sort::sort(uint32_t* keys1, uint32_t* indices1, size_t count) {
  if(count != capacity) {
    if(capacity) {
      cudaFree(keys2);
      cudaFree(indices2);
      cudaFree(aux);
    }

    if(count) {
      cudaMalloc((void**)&keys2, sizeof(uint32_t) * count);
      cudaMalloc((void**)&indices2, sizeof(uint32_t) * count);

      cub::DoubleBuffer<uint32_t> keys(keys1, keys2);
      cub::DoubleBuffer<uint32_t> indices(indices1, indices2);
      cub::DeviceRadixSort::SortPairs(nullptr, aux_bytes, keys, indices,
        count, 0, 30);

      cudaMalloc((void**)&aux, aux_bytes);
      capacity = count;
    }
  }

  if(count) {
    // Fill the indices1 buffer with natural numbers.
    transform<true>([=](int index) {
      indices1[index] = index;
    }, count);

    cub::DoubleBuffer<uint32_t> keys(keys1, keys2);
    cub::DoubleBuffer<uint32_t> indices(indices1, indices2);
    cub::DeviceRadixSort::SortPairs(aux, aux_bytes, keys, indices, count, 
      0, 30);

    if(keys.Current() != keys1) {
      // Copy the current buffer back to the OpenGL buffer.
      cudaMemcpy(keys1, keys.Current(), sizeof(uint32_t) * count,
        cudaMemcpyDeviceToDevice);
      cudaMemcpy(indices1, indices.Current(), sizeof(uint32_t) * count,
        cudaMemcpyDeviceToDevice);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


// Simulation parameters are stored in host memory in system_t kept in UBO 1
// to support shaders.
struct SimParams {
  // Particle characteristics.
  int   numBodies         = 30000;
  float particleRadius    = 1.f / 64;

  // Particle distribution. This world box is always centered at the origin.
  vec3  worldSize         = vec3(2, 2, 1.5);
  vec3  cellSize          = 0;
  ivec3 gridSize          = 0;

  // Integration.
  vec3  gravity           = vec3(0, -.0003, 0);
  float deltaTime         = 0.3f;
  float globalDamping     = 1;

  // Physics.
  float spring            = 0.5f;
  float damping           = 0.02f;
  float shear             = 0.1f;
  float attraction        = 0;
  float boundaryDamping   = -0.5f;

  // TODO: The wrecking ball.
  vec3  colliderPos       = vec3(-1.2, -0.8, 0.8);
  float colliderRadius    = 0.2f;

  // Rendering parameters.
  mat4 view               = mat4();
  mat4 proj               = mat4();
  float pointScale        = 0;
  float pointRadius       = 0.0625f;
  float fov               = radians(60.0f);

  int sort_backend        = 0;
  int collide_backend     = 0;
  int integrate_backend   = 0;

  vec3 worldMax() const noexcept { return  worldSize / 2; }
  vec3 worldMin() const noexcept { return -worldSize / 2; }

  int numCells() const noexcept {
    return gridSize.x * gridSize.y * gridSize.z;
  }

  int cellHash(ivec3 cell) const noexcept {
    return cell.x + gridSize.x * (cell.y + gridSize.y * cell.z);
  }
};

// Park the simulation parameters at ubo 1 and keep it there throughout the
// frame. UBO 0 is reserved for gl_transform.
[[spirv::uniform(1)]]
SimParams sim_params_ubo;

inline vec3 collide_spheres(vec3 posA, vec3 posB, vec3 velA, vec3 velB,
  float radiusA, float radiusB, const SimParams& params) {

  vec3 relPos = posB - posA;
  float dist = length(relPos);
  float collideDist = radiusA + radiusB;

  vec3 force { };
  if(dist < collideDist) {
    vec3 norm = relPos / dist;

    // relative velocity.
    vec3 relVel = velB - velA;

    // relative tangential velocity.
    vec3 tanVel = relVel - dot(relVel, relVel) * norm;

    // spring force.
    force = -params.spring * (collideDist - dist) * norm;
    
    // dashpot (damping) fgorce
    force += params.damping * relVel;

    // tangential shear force
    force += params.shear * tanVel;

    // attraction
    force += params.attraction * relPos;
  }

  return force;
}

inline ivec3 calcGridPos(vec3 p, const SimParams& params) {
  return (ivec3)floor((p - params.worldMin()) / params.cellSize);
}

inline int hashGridPos(ivec3 p, const SimParams& params) {
  p = clamp(p, ivec3(0), params.gridSize - 1);
  return params.cellHash(p);
}

struct system_t {
  system_t(SimParams params);
  
  // The reset writes count number of particles to the end of the array.
  // This must be <= numBodies.
  void reset();
  void init_grid(int count);

  void resize(bool clear = false);

  void update(float deltaTime);

  template<bool use_cuda> 
  void integrate();

  template<bool use_cuda> 
  void sort_particles();

  template<bool use_cuda> 
  void collide();

  // Host and device copies of SimParams.
  SimParams params;  
  gl_buffer_t<const SimParams> params_ubo;

  hybrid_buffer_t<vec4[]> positions;
  hybrid_buffer_t<vec4[]> velocities;
  hybrid_buffer_t<vec4[]> positions_out;
  hybrid_buffer_t<vec4[]> velocities_out;

  // Hash each particle to a cell ID.
  hybrid_buffer_t<uint32_t[]> cell_hash;

  // When sorting by cell hash, generate these particle indices to help
  // reorder the buffers.
  hybrid_buffer_t<uint32_t[]> gather_indices;

  // Keep the min and max particle index for each cell.
  hybrid_buffer_t<ivec2[]> cell_ranges;

  // Cache of buffers for merge sort.
  mergesort_pipeline_t<uint32_t, uint32_t> opengl_sort;
  cub_radix_sort cuda_sort;
};

inline float frand(float range) {
  return (range / RAND_MAX) * rand();
}
inline float frand(float min, float max) {
  return min + frand(max - min);
}
inline float frand() {
  return frand(1);
}
inline vec3 frand3(float r) {
  return vec3(frand(-r, r), frand(-r, r), frand(-r, r));
}

system_t::system_t(SimParams params) : params(params) {
  reset();
}

void system_t::resize(bool clear) {
  int num_particles = params.numBodies;
  int old_particles = positions.size();

  // Resize the buffers according to the new particle count.
  if(clear || num_particles != old_particles) {
    positions.resize(num_particles, true);
    velocities.resize(num_particles, true);
    positions_out.resize(num_particles);
    velocities_out.resize(num_particles);
    cell_hash.resize(num_particles);
    gather_indices.resize(num_particles);
  }

  // Compute an optimal grid size.
  float diam = 2 * params.particleRadius;
  params.gridSize = max(1, ivec3(floor(params.worldSize / diam)));
  params.cellSize = params.worldSize / (vec3)params.gridSize;
  cell_ranges.resize(params.numCells());

  if(clear)
    init_grid(num_particles);
  else if(num_particles > old_particles)
    init_grid(num_particles - old_particles);
}

void system_t::reset() {
  resize(true);
}

void system_t::init_grid(int count) {
  int s = (int)ceil(powf((float)count, 1.f / 3));
  float spacing = 2 * params.particleRadius;
  float jitter = .1f * params.particleRadius;

  int num_particles = params.numBodies;
  int first = num_particles - count;

  float r = params.particleRadius;
  float coef = 1.f / count;

  float extent = spacing * s + jitter;
  vec3 center = vec3(
    (params.worldSize.x - extent) / 2,   // center in x
     params.worldSize.y - extent,        // place at the top in y
    (params.worldSize.z - extent) / 2    // center in z
  ) + params.worldMin();

  std::vector<vec4> pos_host(count);
  std::vector<vec4> vel_host(count);
  for(int z = 0, index = 0; z < s; ++z) {
    for(int y = 0; y < s; ++y) {
      for(int x = 0; x < s && index < count; ++x, ++index) {
        vec3 pos = spacing * vec3(x, y, z) + r + frand(jitter) + center;
        pos_host[index] = vec4(pos, coef * index);

        // Give the particle some downward velocity.
        vel_host[index] = vec4(0, -.03, 0, 0);
      }
    }
  }

  positions.set_data_range(pos_host.data(), first, count);
  velocities.set_data_range(vel_host.data(), first, count);
}

void system_t::update(float deltaTime) {
  // Reorder the particles so that we can perform fast collision detection.
  if(1 == params.sort_backend)
    sort_particles<true>();
  else
    sort_particles<false>();

  // Perform collision to accumulate forces on each particles.
  // This is the physics part.
  if(1 == params.collide_backend)
    collide<true>();
  else
    collide<false>();

  // Advance the velocities and positions.
  if(1 == params.integrate_backend)
    integrate<true>();
  else
    integrate<false>();
}

template<bool use_cuda>
void system_t::sort_particles() {
  int num_particles = params.numBodies;

  // Hash particles into cells.
  auto pos_data = positions.bind<0, use_cuda>();
  auto hash_data = cell_hash.bind<1, use_cuda>();

  // 1. Quantize the particles into cells. Hash the cell coordinates
  //    into an integer.
  transform<use_cuda>([=, params=params](int index) {
    vec3 pos = pos_data[index].xyz;
    ivec3 gridPos = calcGridPos(pos, params);
    int hash = hashGridPos(gridPos, params);

    hash_data[index] = hash;

  }, num_particles);

  positions.unbind<0, use_cuda>();
  cell_hash.unbind<1, use_cuda>();

  // 2. Sort the particles by their hash. The value of the sort is the index
  //    of the particle.
  if constexpr(use_cuda) {
    uint32_t* keys = cell_hash.bind<0, true>();
    uint32_t* indices = gather_indices.bind<1, true>();

    cuda_sort.sort(keys, indices, num_particles);

    cell_hash.unbind<0, true>();
    gather_indices.unbind<1, true>();

  } else {
    opengl_sort.sort_keys_indices(cell_hash.gl_buffer, 
      gather_indices.gl_buffer, num_particles); 
  }

  // 3. Reorder the particles according to their gather indices.
  auto pos_in = positions.bind<0, use_cuda>();
  auto vel_in = velocities.bind<1, use_cuda>();
  auto hash_in = cell_hash.bind<2, use_cuda>();
  auto gather_in = gather_indices.bind<3, use_cuda>();
  auto pos_out = positions_out.bind<4, use_cuda>();
  auto vel_out = velocities_out.bind<5, use_cuda>();

  // Clear the ranges array because we'll never visit cells with no
  // particles.  
  cell_ranges.clear_bytes();
  auto cell_ranges_out = cell_ranges.bind<6, use_cuda>();

  transform<use_cuda>([=, num_bodies=params.numBodies](int index) {
    // Load the gather and hash values.
    int gather = gather_in[index];
    int hash = hash_in[index];
    int hash_prev = index ? hash_in[index - 1] : -1;

    // Load the particle data.
    vec4 pos = pos_in[gather];
    vec4 vel = vel_in[gather];

    // Write the cell ranges.
    if(hash_prev < hash) {
      if(index) cell_ranges_out[hash_prev].y = index;
      cell_ranges_out[hash].x = index;
    }

    if(index == num_bodies - 1)
      cell_ranges_out[hash].y = num_bodies;

    // Write the particles to memory.
    pos_out[index] = pos;
    vel_out[index] = vel;

  }, num_particles);

  positions.unbind<0, use_cuda>();
  velocities.unbind<1, use_cuda>();
  cell_hash.unbind<2, use_cuda>();
  gather_indices.unbind<3, use_cuda>();
  positions_out.unbind<4, use_cuda>();
  velocities_out.unbind<5, use_cuda>();

  // Swap the old containers with the new ones.
  positions.swap(positions_out);
  velocities.swap(velocities_out);
}

template<bool use_cuda>
void system_t::collide() {
  auto pos_in = positions.bind<0, use_cuda>();
  auto vel_in = velocities.bind<1, use_cuda>();
  auto cell_ranges_in = cell_ranges.bind<2, use_cuda>();
  auto vel_out = velocities_out.bind<3, use_cuda>();

  transform<use_cuda>([=, params=params](int index) {
    vec3 f { };
    float r = params.particleRadius;

    // Read particle data.
    vec3 pos = pos_in[index].xyz;
    vec3 vel = vel_in[index].xyz;

    // Hash to the grid.
    ivec3 gridPos = calcGridPos(pos, params);

    // Examine neighbouring cells.
    for(int z = -1; z <= 1; ++z) {
      for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
          int hash = hashGridPos(gridPos + ivec3(x, y, z), params);

          // Get the range of particles for this cell.
          ivec2 range = cell_ranges_in[hash];

          // Visit each particle in the cell.
          for(int i = range.x; i < range.y; ++i) {
            
            // Don't collide with one's self.
            if(i != index) {
              vec3 pos2 = pos_in[i].xyz;
              vec3 vel2 = vel_in[i].xyz; 
              
              // Compute the force on the left particle.
              f += collide_spheres(pos, pos2, vel, vel2, r, r, params);
            }
          }
        }
      }
    }
    
    // Integrate the velocity by the new acceleration and write out.
    vel += f; 
    vel_out[index] = vec4(vel, 0);

  }, params.numBodies);

  positions.unbind<0, use_cuda>();
  velocities.unbind<1, use_cuda>();
  cell_ranges.unbind<2, use_cuda>();
  velocities_out.unbind<3, use_cuda>();

  velocities.swap(velocities_out);
}

template<bool use_cuda>
void system_t::integrate() {
  auto pos_data = positions.bind<0, use_cuda>();
  auto vel_data = velocities.bind<1, use_cuda>();

  transform<use_cuda>([=, params=params](int index) {
    // Load the particle.
    vec4 pos4 = pos_data[index];
    vec4 vel4 = vel_data[index];

    vec3 pos = pos4.xyz;
    vec3 vel = vel4.xyz;

    // Apply gravity and damping.
    vel += params.gravity;
    vel *= params.globalDamping;

    // Integrate the position.
    pos += vel * params.deltaTime;

    // Collide with the cube sides.
    vec3 min = params.worldMin() + params.particleRadius;
    bvec3 clip_min = pos < min;
    pos = clip_min ? min : pos;
    vel *= clip_min ? params.boundaryDamping : 1;

    vec3 max = params.worldMax() - params.particleRadius;
    bvec3 clip_max = pos > max;
    pos = clip_max ? max : pos;
    vel *= clip_max ? params.boundaryDamping : 1;

    // Store updated terms.
    pos_data[index] = vec4(pos, pos4.w);
    vel_data[index] = vec4(vel, vel4.w);

  }, params.numBodies);

  positions.unbind<0, use_cuda>();
  velocities.unbind<1, use_cuda>();
}

////////////////////////////////////////////////////////////////////////////////

inline vec3 color_ramp(float t) {
  const int ncolors = 6;
  const vec3 c[ncolors + 1] {
    1, 0, 0,
    1, 1, 0,
    0, 1, 0, 
    0, 1, 1,
    0, 0, 1, 
    1, 0, 1,
    1, 0, 0,
  };

  t *= ncolors;
  int i = (int)floor(t);
  float u = t - i;

  return mix(c[i], c[i + 1], u);
}

[[spirv::vert]]
void vert_shader() {
  vec4 pos = shader_readonly<0, vec4[]>[glvert_VertexID];
  vec4 posEye = sim_params_ubo.view * vec4(pos.xyz, 1);
  
  float dist = length(posEye);
  glvert_Output.PointSize = sim_params_ubo.pointRadius * 
    sim_params_ubo.pointScale / dist;
  glvert_Output.Position = sim_params_ubo.proj * posEye;

  // Pass the color through.
  shader_out<0, vec4> = vec4(color_ramp(pos.w), 1);
}

[[spirv::frag]]
void frag_shader() {
  constexpr vec3 light_dir(.577, .577, .577);
  
  // Scale the point into a (-1, +1) square.
  vec2 pos = vec2(2, -2) * glfrag_PointCoord + vec2(-1, 1);
  float mag2 = dot(pos, pos);
  if(mag2 > 1)
    glfrag_discard();

  vec3 N(pos, sqrt(1 - mag2));

  float diffuse = max(0.f, dot(light_dir, N));
  shader_out<0, vec4> = shader_in<0, vec4> * diffuse;
}

[[spirv::vert]]
void vert_lines() {
  vec3 v = shader_in<0, vec3>;
  vec4 pos(mix(sim_params_ubo.worldMin(), sim_params_ubo.worldMax(), v), 1);
  glvert_Output.Position = sim_params_ubo.proj * (sim_params_ubo.view * pos);
}

[[spirv::frag]]
void frag_lines() {
  shader_out<0, vec4> = 0;
}

struct myapp_t : app_t {
  myapp_t();
  void display() override;
  void configure();

  // Simulation data.
  std::unique_ptr<system_t> system;

  // GL rendering.
  GLuint spheres_program, lines_program;
  GLuint spheres_vao, lines_vao;
};


myapp_t::myapp_t() : app_t("Particles simulation", 800, 600) { 
  camera.distance = 3;
  camera.yaw = radians(90.f);

  // Create the shaders.
  GLuint vs1 = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs1 = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);

  GLuint shaders[] { vs1, fs1, vs2, fs2 };
  glShaderBinary(4, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, 
    __spirv_data, __spirv_size);

  glSpecializeShader(vs1, @spirv(vert_shader), 0, nullptr, nullptr);
  glSpecializeShader(fs1, @spirv(frag_shader), 0, nullptr, nullptr);
  glSpecializeShader(vs2, @spirv(vert_lines), 0, nullptr, nullptr);
  glSpecializeShader(fs2, @spirv(frag_lines), 0, nullptr, nullptr);

  // Render the spheres.
  spheres_program = glCreateProgram();
  glAttachShader(spheres_program, vs1);
  glAttachShader(spheres_program, fs1);
  glLinkProgram(spheres_program);

  glCreateVertexArrays(1, &spheres_vao);

  // Render the box lines.
  lines_program = glCreateProgram();
  glAttachShader(lines_program, vs2);
  glAttachShader(lines_program, fs2);
  glLinkProgram(lines_program);

  // VBO for the box lines.
  const vec3 box_lines[2 * 12] {
    // Left face edges to right face.
    0, 0, 0, 1, 0, 0,
    0, 0, 1, 1, 0, 1,
    0, 1, 0, 1, 1, 0,
    0, 1, 1, 1, 1, 1,

    // Left face connections.
    0, 0, 0, 0, 0, 1,
    0, 0, 1, 0, 1, 1,
    0, 1, 1, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 

    // Right face connections.
    1, 0, 0, 1, 0, 1,
    1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0,
    1, 1, 0, 1, 0, 0, 
  };  

  GLuint vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(box_lines), box_lines, 0);

  glCreateVertexArrays(1, &lines_vao);
  glVertexArrayVertexBuffer(lines_vao, 0, vbo, 0, sizeof(vec3));
  glEnableVertexArrayAttrib(lines_vao, 0);
  glVertexArrayAttribBinding(lines_vao, 0, 0);
  glVertexArrayAttribFormat(lines_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // Initialize a system.
  system = std::make_unique<system_t>(SimParams { });
}

void myapp_t::display() {
  configure();

  SimParams& params = system->params;

  // Set the view matrix.
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  params.proj = camera.get_perspective(width, height);
  params.view = camera.get_view();

  params.fov = camera.fov;
  params.pointScale = .5f * height / tanf(params.fov * .5f);
  
  // Check if particles have been added or removed.
  system->resize();

  // Upload and bind the simulation parameters to UBO=1.
  system->params_ubo.set_data(params);
  system->params_ubo.bind_ubo(1);

  // Clear the background.
  const float bg[4] { .75f, .75f, .75f, 1.0f };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  // Render the spheres.
  // Set the context for point rendering.
  glEnable(GL_PROGRAM_POINT_SIZE);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glUseProgram(spheres_program);
  glBindVertexArray(spheres_vao);
  system->positions.gl_buffer.bind_ssbo(0);
  
  for(int i = 1; i < 7; ++i)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, 0);
  
  glDrawArrays(GL_POINTS, 0, params.numBodies);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glDisable(GL_PROGRAM_POINT_SIZE);

  // Render the box lines.
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glUseProgram(lines_program);
  glBindVertexArray(lines_vao);
  glLineWidth(3.f);
  glDrawArrays(GL_LINES, 0, 24);

  // Integrate for the next frame.
  system->update(.1);
}

void myapp_t::configure() {
  SimParams& params = system->params;

  // Set ImGui to control system parameters.
  ImGui::Begin("particles simluation");

    ImGui::SliderInt("num bodies", &params.numBodies, 1, 65536);
    ImGui::SliderFloat3("box size", &params.worldSize.x, .1, 3);
    ImGui::SliderFloat("time step", &params.deltaTime, 0, 1);

    ImGui::SliderFloat("spring", &params.spring, 0, 1);
    ImGui::SliderFloat("damping", &params.damping, 0, .1f);
    ImGui::SliderFloat("shear", &params.shear, 0, 1);
    ImGui::SliderFloat("attraction", &params.attraction, 0, .1);
    ImGui::SliderFloat("boundary damping", &params.boundaryDamping, -1, 0);

    const char* backends[] { "OpenGL", "CUDA" };
    ImGui::Combo("Sort backend", &params.sort_backend, backends, 2);
    ImGui::Combo("Collide backend", &params.collide_backend, backends, 2);
    ImGui::Combo("Integrate backend", &params.integrate_backend, backends, 2);

    if(ImGui::Button("New Cube"))
      system->reset();

    if(ImGui::Button("Reset")) {
      system->params = SimParams();
      system->reset();
    }

  ImGui::End();
}

int main() { 
  glfwInit();
  gl3wInit();

  myapp_t app;
  app.loop();

  return 0;
}