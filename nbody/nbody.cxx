#define USE_IMGUI

#include "appglfw.hxx"
#include "tipsy.h"
#include <random>
#include <memory>

struct uniforms_t {
  mat4 proj;
  mat4 view;
  vec3 eye;

  vec3 star_color = vec3(1, .6, .3);

  float dt = .0016;
  float damping = 1.f;
  float softening = .1f;
  float point_size = 3.f;
  float cluster_scale = 1.4;
  float velocity_scale = 11;
};

// The uniform buffer is bound for integration and rendering.
[[using spirv: uniform, binding(0)]]
uniforms_t uniforms;

////////////////////////////////////////////////////////////////////////////////
// Integration
// Loop over all input positions and velocities and write new output 
// positions and velocities.

[[using spirv: buffer, binding(0)]]
vec4 buffer_pos_in[];

[[using spirv: buffer, binding(1)]]
vec4 buffer_pos_out[];

[[using spirv: buffer, binding(2)]]
vec4 buffer_vel[];

// Return the force on a from the influence of b.
inline vec3 interaction(vec3 a, vec4 b) {
  vec3 r = b.xyz - a;

  float softeningSq = uniforms.softening * uniforms.softening;
  float dist2 = dot(r, r) + softeningSq;
  float invDist = inversesqrt(dist2);
  float invDistCube = invDist * invDist * invDist;

  float s = b.w * invDistCube;
  return s * r;
}

const int NT = 128;

[[using spirv: comp, local_size(NT)]] 
void integrate_shader() {
  int tid = glcomp_LocalInvocationID.x;
  int gid = glcomp_GlobalInvocationID.x;

  // Query the length of the mapped position buffer for particle count.
  int num_particles = buffer_vel.length;
  int num_tiles = glcomp_NumWorkGroups.x;

  // Load the position for this thread.
  vec4 pos { };
  if(gid < num_particles)
    pos = buffer_pos_in[gid];

  // Compute the total acceleration on pos.
  vec3 acc { };
  for(int tile = 0; tile < num_tiles; ++tile) {
    // Buffer the next NT particles through shared memory.
    [[spirv::shared]] vec4 cache[NT];
    int index2 = NT * tile + tid;
    cache[tid] = index2 < num_particles ? buffer_pos_in[index2] : vec4();
    glcomp_barrier();

    // Use @meta for to unroll all NT number of particle interactions.
    @meta for(int j = 0; j < NT; ++j)
      acc += interaction(pos.xyz, cache[j]);    

    // Once all threads complete, go to the next tile.
    glcomp_barrier();
  }

  if(gid < num_particles) {
    // Load the velocity for this thread.
    vec4 vel = buffer_vel[gid];

    // Update the velocity and position.
    // Draw the particle back to the center.
    vel.xyz += uniforms.dt * acc;
    vel.xyz *= uniforms.damping;

    pos.xyz += uniforms.dt * vel.xyz;

    // Store the updated position and velocity.
    buffer_pos_out[gid] = pos;
    buffer_vel[gid] = vel;
  }
}


////////////////////////////////////////////////////////////////////////////////
// Rendering.

[[using spirv: uniform, binding(0)]]
sampler2D frag_texture;

[[spirv::vert]]
void vert_shader() {
  // vertex.w normally holds the particle's mass. Replace that with 1 for
  // projection.
  vec4 vertex = vec4(shader_in<0, vec3>, 1);
  vec4 pos = uniforms.view * vertex;
  
  glvert_Output.PointSize = max(1.f, 200 * uniforms.point_size / (1 - pos.z));
  glvert_Output.Position = uniforms.proj * pos;
}

[[spirv::frag]]
void frag_shader() {
  shader_out<0, vec4> = 
    vec4(uniforms.star_color, 1) *
    texture(frag_texture, glfrag_PointCoord);
}

////////////////////////////////////////////////////////////////////////////////

// Manage buffers for 
struct system_t {
  system_t(size_t num_particles);
  ~system_t();

  size_t num_particles;

  // Storage buffers for positions and velocities.
  GLuint pos_buffer[2], vel_buffer;
  GLuint vao[2];
  int active = 0;
};

system_t::system_t(size_t num_particles) : num_particles(num_particles) {
  // Allocate particle buffer. This holds 2x the number of particles.
  glCreateBuffers(2, pos_buffer);
  glNamedBufferStorage(pos_buffer[0], sizeof(vec4) * num_particles, 
    nullptr, GL_DYNAMIC_STORAGE_BIT);
  glNamedBufferStorage(pos_buffer[1], sizeof(vec4) * num_particles, 
    nullptr, GL_DYNAMIC_STORAGE_BIT);

  // Allocate velocities buffer. This holds 1x the number of particles.
  glCreateBuffers(1, &vel_buffer);
  glNamedBufferStorage(vel_buffer, sizeof(vec4) * num_particles, 
    nullptr, GL_DYNAMIC_STORAGE_BIT);

  // Create a VAO for the positions.
  glCreateVertexArrays(2, vao);

  for(int i = 0; i < 2; ++i) {
    // Associate storage buffer with vertex binding index 0.
    glVertexArrayVertexBuffer(vao[i], 0, pos_buffer[i], 0, sizeof(vec4));

    // Associate vertex binding index 0 with vertex attribute 0.
    glVertexArrayAttribBinding(vao[i], 0, 0);

    // Describe the representation of the vertex attribute.
    glVertexArrayAttribFormat(vao[i], 0, 4, GL_FLOAT, GL_FALSE, 0);

    // Enable vertex attribute 0.
    glEnableVertexArrayAttrib(vao[i], 0);
  }
}

system_t::~system_t() {
  glDeleteVertexArrays(2, vao);
  glDeleteBuffers(2, pos_buffer);
  glDeleteBuffers(1, &vel_buffer);
}

////////////////////////////////////////////////////////////////////////////////

enum nbody_config_t {
  nbody_config_random,
  nbody_config_shell,
  nbody_config_expand,
};


struct NBodyParams {
  float distance;
  float dt;
  float cluster_scale;
  float velocity_scale;
  float softening;
  float damping;
  float point_size;
  float x, y, z;
};

static const NBodyParams demoParams[] = {
  { 2.0f, 0.010f, 1.54f, 8.0f, 0.1f, 1.0f, 1.0f, 0, -2, -100},
  { 2.0f, 0.010f, 0.68f, 20.0f, 0.1f, 1.0f, 0.8f, 0, -2, -30},
  { 2.0f, 0.0006f, 0.16f, 1000.0f, 1.0f, 1.0f, 0.07f, 0, 0, -1.5f},
  { 2.0f, 0.0006f, 0.16f, 1000.0f, 1.0f, 1.0f, 0.07f, 0, 0, -1.5f},
  { 2.0f, 0.0019f, 0.32f, 276.0f, 1.0f, 1.0f, 0.07f, 0, 0, -5},
  { 2.0f, 0.0016f, 0.32f, 272.0f, 0.145f, 1.0f, 0.08f, 0, 0, -5},
  { 2.0f, 0.016000f, 6.040000f, 0.000000f, 1.000000f, 1.000000f, 0.760000f, 0, 0, -50},
};

const int NumDemos = demoParams.length;

struct myapp_t : app_t {
  myapp_t(int num_particles);
  ~myapp_t();

  void set_demo_params(int active);

  void display() override;
  void key_callback(int key, int scancode, int action, int mods) override;

  void init_texture(int size);
  void init_shaders();
  void init_ubo();

  void reset_positions(int num_particles, nbody_config_t config);
  void reset_random(int num_particles, vec4* positions, vec4* velocities);
  void reset_shell(int num_particles, vec4* positions, vec4* velocities);
  void reset_expand(int num_particles, vec4* positions, vec4* velocities);

  void reset_tipsy(const char* path);

  void configure();
  void update_uniforms();
  void advance();
  void render();

  std::unique_ptr<system_t> system;
  int active_demo = 3;
  int particle_count;

  uniforms_t uniforms;
  GLuint ubo;

  GLuint integrate_program;
  GLuint draw_program;

  GLuint gaussian_texture;

  std::mt19937 mt19937;
};

myapp_t::myapp_t(int num_particles) : app_t("nbody simulation", 1280, 720) { 
  particle_count = num_particles;
  init_texture(32);
  init_shaders();
  init_ubo();
  
  set_demo_params(active_demo);
  reset_positions(num_particles, nbody_config_shell);
}

myapp_t::~myapp_t() {
  glDeleteTextures(1, &gaussian_texture);
  glDeleteBuffers(1, &ubo);
  glDeleteProgram(integrate_program);
  glDeleteProgram(draw_program);
}

void myapp_t::set_demo_params(int active) {
  NBodyParams params = demoParams[active];
  camera.distance = params.distance;
  camera.pitch = camera.yaw = 0;
  uniforms.dt = params.dt;
  uniforms.cluster_scale = params.cluster_scale;
  uniforms.velocity_scale = params.velocity_scale;
  uniforms.softening = params.softening;
  uniforms.damping = params.damping;
  uniforms.point_size = params.point_size;
}

////////////////////////////////////////////////////////////////////////////////

float evalHermite(float pA, float pB, float vA, float vB, float u) {
  float u2 =(u*u), u3=u2*u;
  float B0 = 2*u3 - 3*u2 + 1;
  float B1 = -2*u3 + 3*u2;
  float B2 = u3 - 2*u2 + u;
  float B3 = u3 - u;
  return (B0*pA + B1*pB + B2*vA + B3*vB);
}

void myapp_t::init_texture(int size) {
  std::vector<uint32_t> tex(size * size);
  for(int row = 0; row < size; ++row) {
    float x = 2.0f * row / (size - 1) - 1;

    for(int col = 0; col < size; ++col) {
      float y = 2.0f * col / (size - 1) - 1;

      float dist = std::min(1.0f, sqrtf(x * x + y * y));
      float hermite = evalHermite(1, 0, 0, 0, dist);

      unsigned char val = (unsigned char)(hermite * 255);
      tex[row * size + col] = val | (val<< 8) | (val<< 16) | (val<< 24);
    }
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &gaussian_texture);
  glTextureParameteri(gaussian_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(gaussian_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(gaussian_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(gaussian_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTextureStorage2D(gaussian_texture, 1, GL_RGBA8, size, size);
  glTextureSubImage2D(gaussian_texture, 0, 0, 0, size, size, GL_RGBA, 
    GL_UNSIGNED_BYTE, tex.data());
  glGenerateTextureMipmap(gaussian_texture);
}

void myapp_t::init_shaders() {
  // Compile the graphics shaders.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
  GLuint shaders[] { vs, fs, cs };
  glShaderBinary(3, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
    __spirv_data, __spirv_size);
  char ErrorLog[1024];
  int compileStatus;

  glSpecializeShader(vs, @spirv(vert_shader), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_shader), 0, nullptr, nullptr);
  glSpecializeShader(cs, @spirv(integrate_shader), 0, nullptr, nullptr);

    char log[10000];
    GLsizei len = 10000;
    glGetShaderInfoLog(cs, 10000, &len, log);
    puts(log);

  draw_program = glCreateProgram();
  glAttachShader(draw_program, vs);
  glAttachShader(draw_program, fs);
  glLinkProgram(draw_program);

  integrate_program = glCreateProgram();
  glAttachShader(integrate_program, cs);
  glLinkProgram(integrate_program);

    glGetProgramInfoLog(integrate_program, 10000, &len, log);
    puts(log);
}

void myapp_t::init_ubo() {
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(uniforms_t), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);
}

////////////////////////////////////////////////////////////////////////////////

void myapp_t::display() { 
  configure();
  update_uniforms();
  advance();
  render();
}

void myapp_t::configure() {
  ImGui::Begin("nbody");
    
    // Scale the timestep by 1000 for more human units.
    float dt = 1000 * uniforms.dt;
    ImGui::DragFloat("timestep",       &dt,             .01f);
    uniforms.dt = dt / 1000;

    ImGui::DragFloat("damping",        &uniforms.damping,        .0001f);
    ImGui::DragFloat("softening",      &uniforms.softening,      .0001f);
    ImGui::DragFloat("point size",     &uniforms.point_size,     .01f);
    ImGui::DragFloat("cluster scale",  &uniforms.cluster_scale,  .1f);
    ImGui::DragFloat("velocity scale", &uniforms.velocity_scale, .1f);

    ImGui::ColorEdit3("star color", &uniforms.star_color.x);

    // Camera position.
    ImGui::DragFloat("distance",       &camera.distance);
    ImGui::SliderFloat("pitch",        &camera.pitch, -M_PI / 2, M_PI / 2);
    ImGui::SliderFloat("yaw",          &camera.yaw, -M_PI, M_PI);

    ImGui::Text("Launch a new system:");

    ImGui::SliderInt("Particles", &particle_count, 1024, 131072);
    
    if(ImGui::Button("Random")) {
      reset_positions(particle_count, nbody_config_random);
      camera.distance = 2;
      uniforms.dt = 0.0006;
      uniforms.point_size = .07;
    }

    ImGui::SameLine();
    if(ImGui::Button("Shell")) {
      reset_positions(particle_count, nbody_config_shell);
      camera.distance = 2;
      uniforms.dt = 0.0006;
      uniforms.point_size = .07;
    }

    ImGui::SameLine();
    if(ImGui::Button("Expand")) {
      reset_positions(particle_count, nbody_config_expand);
      camera.distance = 2;
      uniforms.dt = 0.0006;
      uniforms.point_size = .07;
    }

    ImGui::NewLine();
    if(ImGui::Button("Load galaxy_20K.bin"))
      reset_tipsy("galaxy_20K.bin");

    if(ImGui::Button("Next demo")) {
      active_demo = (active_demo + 1) % NumDemos;
      set_demo_params(active_demo);
      reset_positions(system->num_particles, nbody_config_shell);
    }
    
  ImGui::End();
}

void myapp_t::update_uniforms() {
  // Set the view matrix.
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  uniforms.proj = camera.get_perspective(width, height);
  uniforms.view = camera.get_view();
  uniforms.eye = camera.get_eye();

  glNamedBufferSubData(ubo, 0, sizeof(uniforms_t), &uniforms);
}

void myapp_t::advance() {
  // Integrate particles.
  glUseProgram(integrate_program);

  // Bind the uniform buffer.
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

  // Bind the position and velocity buffers.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 
    system->pos_buffer[system->active]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 
    system->pos_buffer[1 - system->active]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, system->vel_buffer);

  // Dispatch a compute grid.
  int num_groups = (system->num_particles + NT - 1) / NT;
  glDispatchCompute(num_groups, 1, 1);

  // Unbind the storage buffers.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
  glUseProgram(0);

  // Flip the active section of the positions buffer.
  system->active = 1 - system->active;
}

void myapp_t::render() {
  // Render particles.
  const float bg[4] { 0 };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_PROGRAM_POINT_SIZE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  //glEnable(0x8861);

  // Render particles.
  glUseProgram(draw_program);

  // Set the VAO.
  glBindVertexArray(system->vao[system->active]);

  // Set the uniforms.
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

  // Set the texture.
  glBindTextureUnit(0, gaussian_texture);

  // Draw all particles.
  glDrawArrays(GL_POINTS, 0, system->num_particles);

  // Clear the bindings.
  glBindTextureUnit(0, 0);
  glBindVertexArray(0);
  glUseProgram(0);

  glDisable(GL_PROGRAM_POINT_SIZE);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void myapp_t::key_callback(int key, int scancode, int action, int mods) {
  if(GLFW_PRESS == action && GLFW_KEY_R == key) {
    active_demo = (active_demo + 1) % NumDemos;
    set_demo_params(active_demo);
    reset_positions(system->num_particles, nbody_config_shell);
  }
}

vec3 random_on_sphere(std::mt19937& mt19937) {
  std::uniform_real_distribution<float> dist(0, 1);
  float theta = 2 * M_PIf32 * dist(mt19937);
  float y = 2 * dist(mt19937) - 1;
  float zx = sqrt(1 - y * y);

  return vec3(zx * cos(theta), y, zx * sin(theta));
}

vec3 random_in_sphere(std::mt19937& mt19937) {
  std::uniform_real_distribution<float> dist(0, 1);
  float r = sqrt(dist(mt19937));
  return r * random_on_sphere(mt19937);
}

void myapp_t::reset_positions(int num_particles, nbody_config_t config) {
  particle_count = num_particles;
  std::vector<vec4> positions(num_particles);
  std::vector<vec4> velocities(num_particles);

  switch(config) {
    case nbody_config_random: 
      reset_random(num_particles, positions.data(), velocities.data());
      break;

    case nbody_config_shell: 
      reset_shell(num_particles, positions.data(), velocities.data());
      break;

    case nbody_config_expand: 
      reset_expand(num_particles, positions.data(), velocities.data());
      break;
  }

  system.reset(new system_t(num_particles));

  std::string s = "n-body: " + std::to_string(num_particles) + " particles";
  glfwSetWindowTitle(window, s.c_str());

  // Upload seed positions and velocities.
  glNamedBufferSubData(system->pos_buffer[0], 0, sizeof(vec4) * num_particles, 
    positions.data());
  glNamedBufferSubData(system->vel_buffer, 0, sizeof(vec4) * num_particles,
    velocities.data());
}

void myapp_t::reset_random(int num_particles, vec4* positions, 
  vec4* velocities) {

  float scale = uniforms.cluster_scale * std::max(1.f, num_particles / 1024.f);
  float vscale = uniforms.velocity_scale * scale;

  for(int i = 0; i < num_particles; ++i) {
    positions[i] = vec4(scale * random_in_sphere(mt19937), 1);
    velocities[i] = vec4(vscale * random_in_sphere(mt19937), 1);
  }
}

void myapp_t::reset_shell(int num_particles, vec4* positions, 
  vec4* velocities) {

  float scale = uniforms.cluster_scale;
  float vscale = uniforms.velocity_scale * scale;
  float inner = 2.5f * scale;
  float outer = 4.0f * scale;

  std::uniform_real_distribution<float> dist(0, 1);
  for(int i = 0; i < num_particles; ++i) {
    vec3 point = random_on_sphere(mt19937);
    vec3 r2 = inner + (outer - inner) * 
      vec3(dist(mt19937), dist(mt19937), dist(mt19937));
    positions[i] = vec4(r2 * point, 1);

    // Cross the components.
    vec3 axis(0, 0, 1);
    if(1 - dot(point, axis))
      axis = normalize(vec3(point.yx, 1));
    
    vec3 vv = cross(positions[i].xyz, axis);
    velocities[i] = vec4(vscale * vv, 1);
  }
}
void myapp_t::reset_expand(int num_particles, vec4* positions, 
  vec4* velocities) {

  float scale = uniforms.cluster_scale * num_particles / 1024.f;
  float vscale = scale * uniforms.velocity_scale;

  for(int i = 0; i < num_particles; ++i) {
    vec3 point = random_in_sphere(mt19937);
    positions[i] = vec4(scale * point, 1);
    velocities[i] = vec4(vscale * point, 1);
  }
}

void myapp_t::reset_tipsy(const char* filename) {
  std::vector<vec4> positions;
  std::vector<vec4> velocities;
  std::vector<int> bodies;
  int total, first, second, third;
  read_tipsy_file(positions, velocities, bodies, filename, 
    total, first, second, third);

  size_t num_particles = positions.size();
  std::string s = "n-body: " + std::to_string(num_particles) + " particles";
  glfwSetWindowTitle(window, s.c_str());

  particle_count = num_particles;
  system.reset(new system_t(num_particles));

  // Upload seed positions and velocities.
  glNamedBufferSubData(system->pos_buffer[0], 0, sizeof(vec4) * num_particles, 
    positions.data());
  glNamedBufferSubData(system->vel_buffer, 0, sizeof(vec4) * num_particles,
    velocities.data());

  // Reset the camera position.
  camera.distance = 50;
  camera.pitch = -1.2;
  camera.yaw = 0;
  uniforms.dt = .005;
  uniforms.point_size = 1.1;
}

int main() {
  glfwInit();
  gl3wInit();

  myapp_t app(30720);
  app.loop();

  return 0;
}