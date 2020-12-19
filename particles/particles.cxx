#include <mgpu/kernel_mergesort.hxx>
#include <memory>

#define USE_IMGUI
#include "../include/appglfw.hxx"

using mgpu::gl_buffer_t;

// Simulation parameters are stored in host memory in system_t kept in UBO 1
// to support shaders.
struct SimParams {
  // Particle characteristics.
  int   numBodies         = 30000;
  float particleRadius    = 1.f / 64;

  // Particle distribution. This world box is always centered at the origin.
  vec3  worldSize         = vec3(2, 2, 2);
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

  // The wrecking ball.
  vec3  colliderPos       = vec3(-1.2, -0.8, 0.8);
  float colliderRadius    = 0.2f;

  // Rendering parameters.
  mat4 view               = mat4();
  mat4 proj               = mat4();
  float pointScale        = 0;
  float pointRadius       = 0.0625f;
  float fov               = radians(60.0f);

  vec3 worldMin() const noexcept { return -worldSize / 2; }
  vec3 worldMax() const noexcept { return  worldSize / 2; }

  int numCells() const noexcept {
    return gridSize.x * gridSize.y * gridSize.z;
  }

  int cellHash(ivec3 cell) const noexcept {
    return cell.x + gridSize.x * (cell.y + gridSize.y * cell.z);
  }
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
  void integrate();
  void sort_particles();
  void collide();

  // Host and device copies of SimParams.
  SimParams params;  
  gl_buffer_t<const SimParams> params_ubo;

  gl_buffer_t<vec4[]> positions;
  gl_buffer_t<vec4[]> velocities;
  gl_buffer_t<vec4[]> positions_out;
  gl_buffer_t<vec4[]> velocities_out;

  // Hash each particle to a cell ID.
  gl_buffer_t<int[]> cell_hash;

  // When sorting by cell hash, generate these particle indices to help
  // reorder the buffers.
  gl_buffer_t<int[]> gather_indices;

  // Keep the min and max particle index for each cell.
  gl_buffer_t<ivec2[]> cell_ranges;

  // Cache of buffers for merge sort.
  mgpu::mergesort_pipeline_t<int, int> sort_pipeline;
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
  int old_particles = positions.count;

  // Resize the buffers according to the new particle count.
  if(clear || num_particles != old_particles) {
    positions.resize(num_particles, true);
    velocities.resize(num_particles, true);
    positions_out.resize(num_particles);
    velocities_out.resize(num_particles);
    cell_hash.resize(num_particles);
    gather_indices.resize(num_particles);

    // Compute an optimal grid size.
    float diam = 2 * params.particleRadius;
    params.gridSize = max(1, ivec3(floor(params.worldSize / diam)));
    params.cellSize = params.worldSize / (vec3)params.gridSize;

    cell_ranges.resize(params.numCells());
  }

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
      }
    }
  }

  positions.set_data_range(pos_host.data(), first, count);
  velocities.set_data_range(vel_host.data(), first, count);
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
  // Check if particles have been added or removed.
  resize();

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

  // 3. Clear the ranges array because we'll never visit cells with no
  // particles.  
  cell_ranges.clear_bytes();

  // 4. Reorder the particles according to their gather indices.
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto hash_in = cell_hash.bind_ssbo<2>();
  auto gather_in = gather_indices.bind_ssbo<3>();
  auto pos_out = positions_out.bind_ssbo<4>();
  auto vel_out = velocities_out.bind_ssbo<5>();
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
}

void system_t::collide() {
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto cell_ranges_in = cell_ranges.bind_ssbo<2>();

  auto vel_out = velocities_out.bind_ssbo<3>();

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

          // Visit each particle in the cell.
          for(int i = range.x; i < range.y; ++i) {
            
            // Don't collide with one's self.
            if(i != index) {
              vec3 pos2 = pos_in[i].xyz;
              vec3 vel2 = vel_in[i].xyz; 
              
              // Compute the force on the left particle.
              f += collide_spheres(pos, pos2, vel, vel2, r, r, sim_params_ubo);
            }
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

  velocities.swap(velocities_out);
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
}

////////////////////////////////////////////////////////////////////////////////

inline vec3 color_ramp(float t) {
  const int ncolors = 6;
  const vec3 c[ncolors] {
    1, 0, 0,
    1, 1, 0,
    0, 1, 0, 
    0, 1, 1,
    0, 0, 1, 
    1, 0, 1,
  };

  t *= ncolors;
  int i = (int)floor(t);
  float u = t - i;

  return mix(c[i], c[(i + 1) % ncolors], u);
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
  system->positions.bind_ssbo(0);
  
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


    // ImGui::SliderFloat("gravity", &params.gravity.y, -.01, 0);

    ImGui::SliderFloat("spring", &params.spring, 0, 1);
    ImGui::SliderFloat("damping", &params.damping, 0, .1f);
    ImGui::SliderFloat("shear", &params.shear, 0, 1);
    ImGui::SliderFloat("attraction", &params.attraction, 0, .1);
    ImGui::SliderFloat("boundary damping", &params.boundaryDamping, -1, 0);

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