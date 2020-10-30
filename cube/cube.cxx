#include "appglfw.hxx"

#define STB_IMAGE_IMPLEMENTATION
#include "texture.hxx"

[[using spirv: in, location(0)]]
vec3 in_position_vs;

[[using spirv: in, location(1)]]
vec2 in_texcoord_vs;

[[using spirv: out, location(1)]]
vec2 out_texcoord_vs;

[[using spirv: in, location(1)]]
vec2 in_texcoord_fs;

[[using spirv: out, location(0)]]
vec4 out_color_fs;

[[using spirv: uniform, binding(0)]]
sampler2D texture_sampler;

struct uniforms_t {
  mat4 view_proj;
  float seconds;
};

[[using spirv: uniform, binding(0)]]
uniforms_t uniforms;

[[spirv::vert]]
void vert_main() {
  // Create a rotation matrix.
  mat4 rotate = make_rotateY(uniforms.seconds);

  // Rotate the position.
  vec4 position = rotate * vec4(in_position_vs, 1);

  // Write to a builtin variable.
  glvert_Output.Position = uniforms.view_proj * position;

  // Pass texcoord through.
  out_texcoord_vs = in_texcoord_vs;
}

[[spirv::frag]]
void frag_main() {
  // Load the inputs. The locations correspond to the outputs from the
  // vertex shader.
  vec2 texcoord = in_texcoord_fs;
  vec4 color = texture(texture_sampler, texcoord);

  // Write to a variable template.
  out_color_fs = color;
}

////////////////////////////////////////////////////////////////////////////////

struct myapp_t : app_t {
  myapp_t();
  void display() override;
  void create_shaders();
  void create_vao();

  GLuint program;
  GLuint texture;
  GLuint vao;
  GLuint ubo;
};

myapp_t::myapp_t() : app_t("C++ shaders") {
  // Set the camera.
  camera.distance = 4;
  camera.pitch = radians(30.f);

  create_shaders();
  create_vao();
}

void myapp_t::create_shaders() {
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, fs };
  glShaderBinary(2, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
    __spirv_data, __spirv_size);

  glSpecializeShader(vs, @spirv(vert_main), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_main), 0, nullptr, nullptr);

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
}

void myapp_t::create_vao() {
  // Create the vertex array.
  struct vertex_t {
    vec3 pos;
    uint16_t u, v;
  };

  const vertex_t vertices[36] {
    +1, +1, +1, 0x0000, 0xffff, 
    +1, +1, -1, 0x0000, 0x0000, 
    +1, -1, -1, 0xffff, 0x0000, 
    +1, +1, +1, 0x0000, 0xffff, 
    +1, -1, -1, 0xffff, 0x0000, 
    +1, -1, +1, 0xffff, 0xffff, 
    +1, +1, +1, 0x0000, 0xffff, 
    +1, -1, +1, 0x0000, 0x0000, 
    -1, -1, +1, 0xffff, 0x0000, 
    +1, +1, +1, 0x0000, 0xffff, 
    -1, -1, +1, 0xffff, 0x0000, 
    -1, +1, +1, 0xffff, 0xffff, 
    +1, +1, +1, 0x0000, 0xffff, 
    -1, +1, +1, 0x0000, 0x0000, 
    -1, +1, -1, 0xffff, 0x0000, 
    +1, +1, +1, 0x0000, 0xffff, 
    -1, +1, -1, 0xffff, 0x0000, 
    +1, +1, -1, 0xffff, 0xffff, 
    -1, -1, -1, 0x0000, 0xffff, 
    +1, -1, -1, 0x0000, 0x0000, 
    +1, +1, -1, 0xffff, 0x0000, 
    -1, -1, -1, 0x0000, 0xffff, 
    +1, +1, -1, 0xffff, 0x0000, 
    -1, +1, -1, 0xffff, 0xffff, 
    -1, -1, -1, 0x0000, 0xffff, 
    -1, +1, -1, 0x0000, 0x0000, 
    -1, +1, +1, 0xffff, 0x0000, 
    -1, -1, -1, 0x0000, 0xffff, 
    -1, +1, +1, 0xffff, 0x0000, 
    -1, -1, +1, 0xffff, 0xffff, 
    -1, -1, -1, 0x0000, 0xffff, 
    -1, -1, +1, 0x0000, 0x0000, 
    +1, -1, +1, 0xffff, 0x0000, 
    -1, -1, -1, 0x0000, 0xffff, 
    +1, -1, +1, 0xffff, 0x0000, 
    +1, -1, -1, 0xffff, 0xffff, 
  };
  GLuint vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);  

  // Create the VAO.
  // Associate vbo into bindingindex 0.
  glCreateVertexArrays(1, &vao);
  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(vertex_t));

  // Enable vertex attribute 0. Associate with binding index 0.
  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 
    offsetof(vertex_t, pos));

  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribFormat(vao, 1, 2, GL_UNSIGNED_SHORT, 
    GL_TRUE, offsetof(vertex_t, u));

  // Make an OpenGL texture.
  texture = load_texture("../assets/thisisfine.jpg");

  // Make a UBO.
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(uniforms_t), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);
}

void myapp_t::display() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);

  const float bg[4] { .2, 0, 0, 0 };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);
  double timer = glfwGetTime();

  // Set the view matrix.
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  mat4 projection = camera.get_perspective(width, height);
  mat4 view = camera.get_view();

  uniforms_t uniforms;
  uniforms.view_proj = projection * view;
  uniforms.seconds = glfwGetTime();
  glNamedBufferSubData(ubo, 0, sizeof(uniforms), &uniforms);

  // Bind the UBO.
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

  // Bind the texture.
  glBindTextureUnit(0, texture);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
  glfwInit();
  gl3wInit();
  myapp_t app;
  app.loop();
  return 0;
}
