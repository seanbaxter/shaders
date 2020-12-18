#include <mgpu/kernel_mergesort.hxx>
#include <memory>
#include "../include/appglfw.hxx"

using mgpu::gl_buffer_t;

// Simulation parameters are stored in host memory in system_t kept in UBO 1
// to support shaders.
struct SimParams {
  // Particle characteristics.
  int   numBodies         = 64; //16384;
  float particleRadius    = 1.f / 64;

  // Particle distribution.
  ivec3 gridSize          = ivec3(64);
  vec3  worldOrigin       = vec3(-1, -1, -1);
  vec3  cellSize          = 2 * particleRadius;

  // Integration.
  vec3  gravity           = vec3(0, -.0003, 0);
  float deltaTime         = 0.5f;
  float globalDamping     = 1;

  // Physics.
  float spring            = 0.5f;
  float damping           = 0.02f;
  float shear             = 0.1f;
  float attraction        = 0;
  float boundaryDamping   = -0.5f;

  // The wrecking ball.
  vec3  colliderPos       = vec3(-1.2, -0.8, 0.8);
  float colliderRadius    = 0.2f;

  // Rendering parameters.
  mat4 view               = mat4();
  mat4 proj               = mat4();
  float pointScale        = 0;
  float pointRadius       = 0.0625f;
  float fov               = radians(60.0f);
};

// Park the simulation parameters at ubo 1 and keep it there throughout the
// frame. UBO 0 is reserved for gl_transform.
[[using spirv: uniform, binding(1)]]
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
    force = -params.spring * (collideDist - dist);
    
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
  return (ivec3)floor((p - params.worldOrigin) / params.cellSize);
}

inline int hashGridPos(ivec3 p, const SimParams& params) {
  p &= params.gridSize - 1;
  return p.x + params.gridSize.x * (p.y + params.gridSize.y * p.z);
}

struct system_t {
  system_t(const SimParams& params);
  ~system_t();
  
  void reset();
  void init_grid(ivec3 size, float spacing, float jitter);

  // void add_sphere(int start, float* pos, float* vel, int r, float spacing);

  void update(float deltaTime);
  void integrate();
  void sort_particles();
  void collide();

  const SimParams& params;

  gl_buffer_t<vec4[]> positions;
  gl_buffer_t<vec4[]> velocities;
  gl_buffer_t<vec4[]> positions_out;
  gl_buffer_t<vec4[]> velocities_out;

  // Interpolation of hues for rendering.
  gl_buffer_t<vec4[]> colors_buffer;

  // Hash each particle to a cell ID.
  gl_buffer_t<int[]> cell_hash;

  // When sorting by cell hash, generate these particle indices to help
  // reorder the buffers.
  gl_buffer_t<int[]> gather_indices;

  // Keep the min and max particle index for each cell.
  gl_buffer_t<ivec2[]> cell_ranges;

  // Cache of buffers for merge sort.
  mgpu::mergesort_pipeline_t<int, int> sort_pipeline;

  GLuint vao;
};

vec3 color_ramp(float t) {
  const int ncolors = 6;
  static constexpr vec3 c[ncolors] {
    { 1.0, 0.0, 0.0 },
    { 1.0, 0.5, 0.0 },
    { 1.0, 1.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 1.0, 1.0 },
    { 0.0, 0.0, 1.0 },
  };

  t = t * (ncolors - 1);
  int i = (int) t;
  float u = t - floor(t);

  return mix(c[i], c[(i + 1) % ncolors], u);
}

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

system_t::system_t(const SimParams& params) : params(params) {
  int num_particles = params.numBodies;

  positions.resize(num_particles);
  velocities.resize(num_particles);
  positions_out.resize(num_particles);
  velocities_out.resize(num_particles);
  colors_buffer.resize(num_particles);
  cell_hash.resize(num_particles);
  gather_indices.resize(num_particles);

  int num_cells = params.gridSize.x * params.gridSize.y * params.gridSize.z;
  cell_ranges.resize(num_cells);

  // Fill the color buffer.
  std::vector<vec4> colors(num_particles);
  for(int i = 0; i < num_particles; ++i)
    colors[i] = vec4(color_ramp((float)i / num_particles), 1);
  colors_buffer.set_data(colors);

  // Create the VAO that binds the color data.
  glCreateVertexArrays(1, &vao);
  glVertexArrayVertexBuffer(vao, 0, colors_buffer, 0, sizeof(vec4));
  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
  glEnableVertexArrayAttrib(vao, 0);
}

system_t::~system_t() {
  glDeleteVertexArrays(1, &vao);
}

void system_t::reset() {
  // Set the positions to a grid of particles. Reset the velocities to 0.
  int s = (int)ceil(powf((float)params.numBodies, 1.f / 3));
  float r = params.particleRadius;
  init_grid(ivec3(s), 2 * r, .01f * r);
}

void system_t::init_grid(ivec3 size, float spacing, float jitter) {

  spacing *= .7;

  int num_particles = params.numBodies;
  float r = params.particleRadius;

  std::vector<vec4> pos_host(num_particles);
  for(int z = 0, index = 0; z < size.z; ++z) {
    for(int y = 0; y < size.y; ++y) {
      for(int x = 0; x < size.x && index < num_particles; ++x, ++index) {
        vec3 v(x, y, z);
        pos_host[index] = vec4(spacing * v + r + frand3(jitter), 1);
      }
    }
  }

  positions.set_data(pos_host);

  vec4 zero { };
  glClearNamedBufferSubData(velocities, GL_RGBA32F, 0, 
    num_particles * sizeof(vec4), GL_RGBA, GL_FLOAT, &zero);
}

/*
void system_t::add_sphere(int start, int count, int r, float spacing) {
  for(int z = -r, index = 0; z <= r; ++z) {
    for(int y = -r; y <= r; ++y) {
      for(int x = -r; x <= r && index < count; ++x) {
        vec3 pos = spacing * vec3(x, y, z);
        float l = length(pos);

        if(l < params.particleRadius) {

          ++index;
        }
      }
    }
  }
}
*/
void system_t::update(float deltaTime) {
  // Reorder the particles so that we can perform fast collision detection.
  sort_particles();

  // Perform collision to accumulate forces on each particles.
  // This is the physics part.
  collide();

  // Advance the velocities and positions.
  integrate();

}

void system_t::sort_particles() {
  int num_particles = params.numBodies;

  // Hash particles into cells.
  auto pos_data = positions.bind_ssbo<0>();
  auto hash_data = cell_hash.bind_ssbo<1>();

  // 1. Quantize the particles into cells. Hash the cell coordinates
  //    into an integer.
  mgpu::gl_transform([=](int index) {
    vec3 pos = pos_data[index].xyz;
    ivec3 gridPos = calcGridPos(pos, sim_params_ubo);
    int hash = hashGridPos(gridPos, sim_params_ubo);

    hash_data[index] = hash;

  }, num_particles);

  // 2. Sort the particles by their hash. The value of the sort is the index
  //    of the particle.
  sort_pipeline.sort_keys_indices(cell_hash, gather_indices, num_particles);

  {
    auto hash = cell_hash.get_data();
    auto indices = gather_indices.get_data();
    printf("%3d: %3d %3d\n", @range(), hash[:], indices[:])...;
    bool is_sorted = (... && (hash[:] <= hash[1:]));
    printf("hash sorted = %d\n", is_sorted);
  }

  // auto g = gather_indices.get_data();
  // printf("%5d: %5d\n", @range(), g[:])...;
  // exit(0);

  // 3. Reorder the particles according to their gather indices.

  // Clear the ranges array because we'll never visit cells with no
  // particles.  
  ivec2 zero { };
  int cell_count = params.gridSize.x * params.gridSize.y * params.gridSize.z;
  glClearNamedBufferSubData(cell_ranges, GL_RG32I, 0, 
    cell_count * sizeof(ivec2), GL_RG, GL_INT, &zero);

  // Reorder the particles and fill the ranges.
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto hash_in = cell_hash.bind_ssbo<2>();
  auto gather_in = gather_indices.bind_ssbo<3>();
  auto pos_out = positions_out.bind_ssbo<4>();
  auto vel_out = positions_out.bind_ssbo<5>();
  auto cell_ranges_out = cell_ranges.bind_ssbo<6>();

  mgpu::gl_transform([=](int index) {
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

    if(index == sim_params_ubo.numBodies - 1)
      cell_ranges_out[hash].y = sim_params_ubo.numBodies;

    // Write the particles to memory.
    pos_out[index] = pos;
    vel_out[index] = vel;

  }, num_particles);

  // Swap the old containers with the new ones.
  positions.swap(positions_out);
  velocities.swap(velocities_out);

  // Print the ranges.
  // auto ranges = cell_ranges.get_data();
  //for(int i = 0; i < ranges.size(); ++i) {
  //  if(ranges[i].x || ranges[i].y)
  //    printf("%6d: (%5d, %5d)\n", i, ranges[i].x, ranges[i].y);
  //}
  //exit(0);
}

void system_t::collide() {
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto cell_ranges_in = cell_ranges.bind_ssbo<2>();

  auto vel_out = velocities.bind_ssbo<3>();

  mgpu::gl_transform([=](int index) {
    vec3 f { };
    float r = sim_params_ubo.particleRadius;

    // Read particle data.
    vec3 pos = pos_in[index].xyz;
    vec3 vel = vel_in[index].xyz;

    // Hash to the grid.
    ivec3 gridPos = calcGridPos(pos, sim_params_ubo);

    // Examine neighbouring cells.
    for(int z = -1; z <= 1; ++z) {
      for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
          int hash = hashGridPos(gridPos + ivec3(x, y, z), sim_params_ubo);

          // Get the range of particles for this cell.
          ivec2 range = cell_ranges_in[hash];

          for(int i = range.x; i < range.y; ++i) {
            // Visit each particle in the cell.
            vec3 pos2 = pos_in[i].xyz;
            vec3 vel2 = vel_in[i].xyz;

            // Compute the force on the left particle.
            f += collide_spheres(pos, pos2, vel, vel2, r, r, sim_params_ubo);
          }
        }
      }
    }

    // Collide with the cursor sphere.
    f += collide_spheres(pos, sim_params_ubo.colliderPos, vel, vec3(), r, 
      sim_params_ubo.colliderRadius, sim_params_ubo);

    // Integrate the velocity by the new acceleration and write out.
    vel += f * sim_params_ubo.deltaTime;
    vel_out[index] = vec4(vel, 0);

  }, params.numBodies);

  std::vector<vec4> vel = velocities.get_data();
  for(int i = 0; i < vel.size(); ++i)
    printf("%5d: %f %f %f\n", i, vel[i].x, vel[i].y, vel[i].z);

  exit(0);
}

void system_t::integrate() {
  auto pos_data = positions.bind_ssbo<0>();
  auto vel_data = velocities.bind_ssbo<1>();

  mgpu::gl_transform([=](int index) {
    SimParams params = sim_params_ubo;

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
    bvec3 clip_max = pos > 1 - params.particleRadius;
    pos = clip_max ? 1 - params.particleRadius : pos;
    vel *= clip_max ? params.boundaryDamping : 1;

    bvec3 clip_min = pos < -1 + params.particleRadius;
    pos = clip_min ? -1 + params.particleRadius : pos;
    vel *= clip_max ? params.boundaryDamping : 1;

    // Store updated terms.
    pos_data[index] = vec4(pos, pos4.w);
    vel_data[index] = vec4(vel, vel4.w);

  }, params.numBodies);
}

////////////////////////////////////////////////////////////////////////////////

struct myapp_t : app_t {
  myapp_t();
  void display() override;
  void configure();

  SimParams params;
  gl_buffer_t<const SimParams> params_ubo;

  std::unique_ptr<system_t> system;
  GLuint program;
};

[[spirv::vert]]
void vert_shader() {
  vec4 pos = shader_readonly<0, vec4[]>[glvert_VertexID];
  vec4 posEye = sim_params_ubo.view * pos;
  
  float dist = length(posEye);
  glvert_Output.PointSize = sim_params_ubo.pointRadius * 
    sim_params_ubo.pointScale / dist;
  glvert_Output.Position = sim_params_ubo.proj * posEye;

  // Pass the color through.
  shader_out<0, vec4> = shader_in<0, vec4>;
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

myapp_t::myapp_t() : app_t("Particles simulation", 800, 600) { 

  camera.adjust(0, 0, .1);

  // Create the shaders.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, fs };
  glShaderBinary(2, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, 
    __spirv_data, __spirv_size);

  glSpecializeShader(vs, @spirv(vert_shader), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_shader), 0, nullptr, nullptr);

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // Initialize a system.
  system = std::make_unique<system_t>(params);
  system->reset();
}

void myapp_t::display() {
  configure();

  // Set the view matrix.
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  params.proj = camera.get_perspective(width, height);
  params.view = camera.get_view();

  params.fov = camera.fov;
  params.pointScale = height / tanf(params.fov * .5f);

  // Upload and bind the simulation parameters to UBO=1.
  params_ubo.set_data(params);
  params_ubo.bind_ubo(1);

  // Clear the background.
  const float bg[4] { .75f, .75f, .75f, 1.0f };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  // Set the context for point rendering.
  glEnable(GL_PROGRAM_POINT_SIZE);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  glUseProgram(program);
  glBindVertexArray(system->vao);
  system->positions.bind_ssbo(0);

  glDrawArrays(GL_POINTS, 0, params.numBodies);

  glUseProgram(0);
  glDisable(GL_PROGRAM_POINT_SIZE);

  // Integrate for the next frame.
  system->update(.1);
}

void myapp_t::configure() {
  // Set ImGui to control system parameters.
}

int main() { 
  glfwInit();
  gl3wInit();

  myapp_t app;
  app.loop();

  return 0;
}