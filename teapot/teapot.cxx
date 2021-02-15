#define USE_IMGUI
#include "appglfw.hxx"
#include "teapot.h"
#include <vector>

inline float point_segment_distance(vec3 p, vec3 a, vec3 b) {
  b -= a;
  float t = dot(p - a, b) / dot(b, b);
  t = clamp(t, 0.f, 1.f);
  return distance(p, a + t * b);
}

[[using spirv: in, location(0)]]
vec4 vert_pos_in;

// Pass-through vertex shader. Don't project to clip space until tessellation
// evaluation.
[[spirv::vert]]
void vert_shader() {
  glvert_Output.Position = shader_in<0, vec4>;
}

struct tess_constant_t {
  float level = 1;

  float get_level(vec3 a, vec3 b) const noexcept {
    return level;
  }
};

struct tess_distance_t {
  vec3 camera_pos;
  float scale = 10;
  float min_dist = .1f;
  float max_dist = 100.0f;

  float get_level(vec3 a, vec3 b) const noexcept {
    // Find the distance from the camera to the edge.
    float d = point_segment_distance(camera_pos, a, b);

    // Clamp the distance.
    d = clamp(d, min_dist, max_dist);

    return scale / d;
  }
};

struct tess_edge_t {
  vec3 camera_pos;
  float edge_length = 100;
  float screen_width;

  float get_level(vec3 a, vec3 b) const noexcept {
    // Get the distance to the line segment defined by a and b.
    float dist = point_segment_distance(camera_pos, a, b);

    // length of the edge
    float len = distance(a, b);

    // edgeLen is approximate desired size in pixels
    float f = max(len * screen_width / (edge_length * dist), 1.0f);
    return f;
  }
};

template<typename tess_t>
[[spirv::tesc(quads, 16)]]
void tesc_shader() {
  vec3 v00 = gltesc_Input[0].Position.xyz;
  vec3 v10 = gltesc_Input[3].Position.xyz;
  vec3 v01 = gltesc_Input[12].Position.xyz;
  vec3 v11 = gltesc_Input[15].Position.xyz;

  // Make a counter-clockwise pass from OL0 through OL3. See Figure 11.1 
  // in OpenGL 4.6 specification.
  gltesc_LevelOuter[0] = shader_ubo<0, tess_t>.get_level(v01, v00);
  gltesc_LevelOuter[1] = shader_ubo<0, tess_t>.get_level(v00, v10);
  gltesc_LevelOuter[2] = shader_ubo<0, tess_t>.get_level(v10, v11);
  gltesc_LevelOuter[3] = shader_ubo<0, tess_t>.get_level(v11, v01);

  // Average the opposing outer edges for inner edge levels.
  gltesc_LevelInner[0] = .5f * (gltesc_LevelOuter[1] + gltesc_LevelOuter[3]);
  gltesc_LevelInner[1] = .5f * (gltesc_LevelOuter[0] + gltesc_LevelOuter[2]);

  // Load from the patch array at gltesc_InvocationID.
  // Write to the output position for gltesc_InvocationID.
  gltesc_Output.Position = gltesc_Input[gltesc_InvocationID].Position;
}

[[using spirv: uniform, location(0)]]
mat4 mat_view;

// OpenGL requires output and spacing on the evaluation stage.
[[using spirv: tese(quads), output(triangle_ccw), spacing(fractional_even)]]
void tese_shader() {
  float u = gltese_TessCoord.x;
  float v = gltese_TessCoord.y;
  
  const mat4 Bezier(
     1,  0,  0,  0,
    -3,  3,  0,  0,
     3, -6,  3,  0,
    -1,  3, -3,  1
  );

  // Compute Bezier values.
  vec4 uvec = Bezier * vec4(1, u, u * u, u * u * u);
  vec4 vvec = Bezier * vec4(1, v, v * v, v * v * v);

  // Compute Bezier tangents.
  vec4 d_uvec = Bezier * vec4(0, 1, 2 * u, 3 * u * u);
  vec4 d_vvec = Bezier * vec4(0, 1, 2 * v, 3 * v * v); 

  vec3 pos { }, du { }, dv { };
  @meta for(int j = 0; j < 4; ++j) {
    @meta for(int i = 0; i < 4; ++i) {
      pos +=   uvec[i] *   vvec[j] * gltese_Input[4 * j + i].Position.xyz;
      du  += d_uvec[i] *   vvec[j] * gltese_Input[4 * j + i].Position.xyz;
      dv  +=   uvec[i] * d_vvec[j] * gltese_Input[4 * j + i].Position.xyz;
    }
  }

  // Pass the position in model space. Let the geometry shader project
  // into clip space.
  gltese_Output.Position = mat_view * vec4(pos, 1);

  // Use the xz plane angle to define a hue between 0 and 1.
  vec3 normal = normalize(cross(du, dv));
  float hue;
  if(abs(normal.y) > .99f)
    hue = 0;
  else 
    hue = (atan2(normal.x, normal.z) + M_PIf32) / (2 * M_PIf32);

  // Convert hue to RGB.
  shader_out<0, vec3> = clamp(
    vec3(
      abs(hue * 6 - 3) - 1,
      2 - abs(hue * 6 - 2),
      2 - abs(hue * 6 - 4)
    ), 0.f, 1.f
  );
}

[[spirv::frag]]
void frag_shader() {
  shader_out<0, vec4> = vec4(shader_in<0, vec3>, 1);
}

////////////////////////////////////////////////////////////////////////////////

struct myapp_t : app_t {
  myapp_t();

  void display() override;
  void configure() override;

  GLuint programs[3];
  GLuint vao;
  GLuint ubo;

  // Indicate the current program.
  int current = 2;
  tess_constant_t tess_constant;
  tess_distance_t tess_distance;
  tess_edge_t tess_edge;

  double prev_time = 0;
  float rotation_speed = 0;
};

myapp_t::myapp_t() : app_t("Tessellation sample") {
  // Load and compile the shaders.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint ts0 = glCreateShader(GL_TESS_CONTROL_SHADER);
  GLuint ts1 = glCreateShader(GL_TESS_CONTROL_SHADER);
  GLuint ts2 = glCreateShader(GL_TESS_CONTROL_SHADER);
  GLuint es = glCreateShader(GL_TESS_EVALUATION_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, ts0, ts1, ts2, es, fs };

  glShaderBinary(shaders.length, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
    __spirv_data, __spirv_size);

  glSpecializeShader(vs, @spirv(vert_shader), 0, nullptr, nullptr);
  glSpecializeShader(ts0, @spirv(tesc_shader<tess_constant_t>), 0, 
    nullptr, nullptr);
  glSpecializeShader(ts1, @spirv(tesc_shader<tess_distance_t>), 0, 
    nullptr, nullptr);
  glSpecializeShader(ts2, @spirv(tesc_shader<tess_edge_t>), 0, 
     nullptr, nullptr);
  glSpecializeShader(es, @spirv(tese_shader), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_shader), 0, nullptr, nullptr);

  GLuint tesc[3] { ts0, ts1, ts2 };
  for(int i = 0; i < 3; ++i) { 
    programs[i] = glCreateProgram();
    glAttachShader(programs[i], vs);
    glAttachShader(programs[i], tesc[i]);
    glAttachShader(programs[i], es);
    glAttachShader(programs[i], fs);
    glLinkProgram(programs[i]);
  }

  // Initialize the VBO with vertices.
  GLuint vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(TeapotVertices), TeapotVertices, 0);

  // Initialize the IBO with indices.
  GLuint ibo;
  glCreateBuffers(1, &ibo);
  glNamedBufferStorage(ibo, sizeof(TeapotIndices), TeapotIndices, 0);

  // Create the VAO and select the VBO and IBO buffers.
  glCreateVertexArrays(1, &vao);
  glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(vec3));
  glVertexArrayElementBuffer(vao, ibo);

  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  // Create a UBO large enough to hold any of the structures.
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(tess_distance_t), nullptr,
    GL_DYNAMIC_STORAGE_BIT);

  prev_time = glfwGetTime();
}

void myapp_t::display() {
  // Process ImGui settings.
  configure();

  const float bg[4] { .5f, .5f, .5f, 1 };
  glClearBufferfv(GL_COLOR, 0, bg);

  // Setup the device.
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);

  glUseProgram(programs[current]);
  glBindVertexArray(vao);

  double time = glfwGetTime();
  double elapsed = time - prev_time;
  prev_time = time;

  camera.adjust(0, rotation_speed * elapsed, 0);

  mat4 view = camera.get_view();
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  // Put the clip space transform into uniform location 0.
  mat4 perspective = camera.get_perspective(width, height);
  mat4 clip = perspective * view;
  glUniformMatrix4fv(0, 1, false, &clip[0][0]);

  // Bind the tessellation-specific UBO.
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

  // Draw the wireframe.
  glDepthFunc(GL_LEQUAL);
  glLineWidth(1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonOffset(1, -10);

  // Draw all patches.
  glPatchParameteri(GL_PATCH_VERTICES, 16);
  glDrawElements(GL_PATCHES, NumTeapotIndices, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

void myapp_t::configure() {
  ImGui::Begin("Tessellation");
  ImGui::SliderFloat("Rotation speed", &rotation_speed, -2, 2);

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  ImGui::Combo("Metric", &current, "constant\0distance\0edge\0");
  switch(current) {
    case 0:
      ImGui::SliderFloat("Level", &tess_constant.level, 1, 40);
      glNamedBufferSubData(ubo, 0, sizeof(tess_constant_t), &tess_constant);
      break;

    case 1:
      tess_distance.camera_pos = camera.get_eye();
      ImGui::SliderFloat("Scale", &tess_distance.scale, 1, 100);
      glNamedBufferSubData(ubo, 0, sizeof(tess_distance_t), &tess_distance);
      break;

    case 2:
      tess_edge.camera_pos = camera.get_eye();
      tess_edge.screen_width = width;
      ImGui::SliderFloat("Edge length (px)", &tess_edge.edge_length, .5, 100);
      glNamedBufferSubData(ubo, 0, sizeof(tess_edge_t), &tess_edge);
      break;
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
