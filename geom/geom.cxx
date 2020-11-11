#include "appglfw.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "texture.hxx"
#include <random>

enum typename vattrib_t {
  vattrib_position = vec3,
  vattrib_texcoord = vec2,
  vattrib_color    = vec3,
};

[[spirv::vert]]
void vert_main() {
  // Vertex shader pass-through. Use the geometry shader to amply points into
  // cubes, then project vertices into clip space.
  glvert_Output.Position = vec4(shader_in<vattrib_position>, 1);
}

[[using spirv: uniform, location(0)]]
mat4 view_mat;

[[spirv::geom(points, triangle_strip, 24)]]
void geom_main() {
  static constexpr vec3 vertices[6][4] {
    { {  1,  1,  1 }, {  1, -1,  1 }, {  1,  1, -1 }, {  1, -1, -1 } },
    { {  1,  1,  1 }, { -1,  1,  1 }, {  1, -1,  1 }, { -1, -1,  1 } },
    { {  1,  1,  1 }, {  1,  1, -1 }, { -1,  1,  1 }, { -1,  1, -1 } },
    { { -1, -1, -1 }, { -1,  1, -1 }, {  1, -1, -1 }, {  1,  1, -1 } },
    { { -1, -1, -1 }, { -1, -1,  1 }, { -1,  1, -1 }, { -1,  1,  1 } },
    { { -1, -1, -1 }, {  1, -1, -1 }, { -1, -1,  1 }, {  1, -1,  1 } },
  };

  static constexpr vec3 normals[6] {
    { 1,  0,  0 }, {  0,  0,  1 }, { 0,  1,  0 },
    { 0,  0, -1 }, { -1,  0,  0 }, { 0, -1,  0 },
  };

  static constexpr vec2 uv[4] {
    { 0, 1 }, { 1, 1 }, { 0, 0 }, { 1, 0 },
  };

  float phi = radians(30.f);
  float theta = radians(-45.f);

  vec3 light_dir = 1.2f * vec3(
    sin(phi) * cos(theta),
               sin(theta),
    cos(phi) * cos(theta)
  );
  float ambient = .5f;
  float box_size = .4f;

  vec4 position = glgeom_Input[0].Position;

  for(int face = 0; face < 6; ++face) {
    float brightness = clamp(-dot(normals[face], light_dir), ambient, 1.f);

    shader_out<vattrib_color> = brightness * vec3(.8, 1.2, 1.2);

    for(int i = 0; i < 4; ++i) {
      // Create a new vertex and project.
      vec4 vertex = position + vec4(box_size * vertices[face][i], 0);
      glgeom_Output.Position = view_mat * vertex;
      
      // Create a new uv coordinate.
      shader_out<vattrib_texcoord> = uv[i];

      // Emit the vertex.
      glgeom_EmitVertex(); 
    }

    // Finish the triangle strip primitive.
    glgeom_EndPrimitive();    
  }
}

[[spirv::frag]]
void frag_main() {
  vec2 texcoord = shader_in<vattrib_texcoord>;
  vec3 color = shader_in<vattrib_color, vec3>;
  shader_out<0, vec4> = vec4(
    color * texture(shader_sampler<0, sampler2D>, texcoord.xy).xyz, 
    1
  );
}

////////////////////////////////////////////////////////////////////////////////
// Host code.

std::random_device rd;
std::mt19937 mt19937(rd());

vec3 new_random_vel(float speed, float angle) {
  std::uniform_real_distribution<float> dist(0, 1);
  float theta = dist(mt19937) * (2 * M_PI);
  float phi = dist(mt19937) * angle;

  float x = speed * sin(phi) * cos(theta);
  float y = speed * cos(phi);
  float z = speed * sin(phi) * sin(theta);

  return vec3(x, y, z);
}

struct particles_t {
  std::vector<vec3> positions;
  std::vector<vec3> velocities;

  float speed = 4;
  float angle = radians(30.f);
  float gravity = 1.f;

  void init_particle(size_t index) noexcept;
  void update(size_t index, float elapsed) noexcept;
  void push_particle();

  size_t num_particles() const noexcept {
    return positions.size();
  }

  void reset();
};

void particles_t::init_particle(size_t index) noexcept {
  positions[index] = vec3(0, -5, 0);
  velocities[index] = new_random_vel(speed, angle);
}

void particles_t::update(size_t index, float elapsed) noexcept {
  vec3& pos = positions[index];
  vec3& vel = velocities[index];

  if(pos.y < -10)
    init_particle(index);

  vel.y -= gravity * elapsed;
  pos += elapsed * vel;
}

void particles_t::push_particle() {
  size_t index = num_particles();
  positions.push_back(vec3(0, 0, 0));
  velocities.push_back(vec3(0, 0, 0));
  init_particle(index);
}

void particles_t::reset() {
  positions.clear();
  velocities.clear();
}

struct myapp_t : app_t {
  myapp_t();
  void display() override;
  void key_callback(int key, int scancode, int action, int mods) override;

  GLuint program;
  GLuint texture;
  GLuint vbo;       // Update with new positions each frame.
  GLuint vao;       // A buffer of positions only.

  particles_t particles;
  size_t max_particles = 500;
  double start_time = 0;
  double prev_time = 0;
};

myapp_t::myapp_t() : app_t("Geometry shader demo") {
  camera.distance = 4;
  camera.yaw = radians(125.f);
  camera.pitch = radians(25.f);

  texture = load_texture("../assets/thisisfine.jpg");

  // Create a VBO.
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, max_particles * sizeof(vec3), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);

  // Create a VAO.
  glCreateVertexArrays(1, &vao);
  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(vec3));

  // Enable vertex attribute 0. Associate with binding index 0.
  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // Load and compile the shaders.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, gs, fs };
  glShaderBinary(3, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
    __spirv_data, __spirv_size);

  glSpecializeShader(vs, @spirv(vert_main), 0, nullptr, nullptr);
  glSpecializeShader(gs, @spirv(geom_main), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_main), 0, nullptr, nullptr);

  // Create the shader program.
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, gs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  start_time = glfwGetTime();
}

void myapp_t::display() {
  const float bg[4] { .2, .3, .6, 0 };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  // Add 10 particles per second until we hit max_particles.
  double cur_time = glfwGetTime() - start_time;
  size_t num_particles = std::min(max_particles, (size_t)(10 * cur_time));

  float elapsed = cur_time - prev_time;
  for(size_t i = 0; i < particles.num_particles(); ++i) {
    // Integrate all existing particles.
    particles.update(i, elapsed);
  }

  for(size_t i = particles.num_particles(); i < num_particles; ++i) {
    // Inject new particles.
    particles.push_particle();
  }

  prev_time = cur_time;

  // Update the VBO with all particle positions.
  glNamedBufferSubData(vbo, 0, num_particles * sizeof(vec3),
    particles.positions.data());

  // Setup the device.
  glUseProgram(program);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);

  // Set the view matrix.
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  mat4 projection = camera.get_perspective(width, height);
  mat4 view = camera.get_view();
  mat4 view_proj = projection * view;
  glUniformMatrix4fv(0, 1, false, &view_proj[0][0]);

  // Render all particles.
  glBindVertexArray(vao);
  glBindTextureUnit(0, texture);
  glDrawArrays(GL_POINTS, 0, num_particles);

  glBindVertexArray(0);
  glUseProgram(0);
}

void myapp_t::key_callback(int key, int scancode, int action, int mods) {
  if(GLFW_PRESS == action && GLFW_KEY_SPACE == key) {
    particles.reset();
    start_time = glfwGetTime();
  }
}

int main() {
  glfwInit();
  gl3wInit();
  myapp_t myapp;
  myapp.loop();

  return 0;
}