#if __circle_build__ < 105
#error "Circle build 105 required to reliably compile this sample"
#endif

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define GL_GLEXT_PROTOTYPES
#include <gl3w/GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <complex>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <csignal>

#include <fstream>
#include "json.hpp"

// Interlacing
#include "adam7.hxx"

namespace imgui {
  // imgui attribute tags.
  using color3   [[attribute]] = void;
  using color4   [[attribute]] = void;
  
  template<typename type_t>
  struct minmax_t {
    type_t min, max;
  };
  using range_float [[attribute]] = minmax_t<float>;
  using range_int   [[attribute]] = minmax_t<int>;

  // Resolution for drag.
  using drag_float [[attribute]] = float;

  using title    [[attribute]] = const char*;
  using url      [[attribute]] = const char*;
}

// Return true if any option has changed.
template<typename options_t>
bool render_imgui(options_t& options, const char* child_name = nullptr) {

  bool changed = false;
  if(!child_name || 
    ImGui::TreeNodeEx(child_name, ImGuiTreeNodeFlags_DefaultOpen)) {

    using namespace imgui;
    if constexpr(@has_attribute(options_t, url))
      ImGui::Text(@attribute(options_t, url));

    @meta for(int i = 0; i < @member_count(options_t); ++i) {{
      typedef @member_type(options_t, i) type_t;
      const char* name = @member_name(options_t, i);
      auto& value = @member_value(options, i);

      if constexpr(@member_has_attribute(options_t, i, color4)) {
        static_assert(std::is_same_v<type_t, vec4>);
        changed |= ImGui::ColorEdit4(name, &value.x);

      } else if constexpr(@member_has_attribute(options_t, i, color3)) {
        static_assert(std::is_same_v<type_t, vec3>);
        changed |= ImGui::ColorEdit3(name, &value.x);

      } else if constexpr(@member_has_attribute(options_t, i, range_float)) {
        auto range = @member_attribute(options_t, i, range_float);

        if constexpr(std::is_same_v<type_t, vec4>) {
          changed |= ImGui::SliderFloat4(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, vec3>) {
          changed |= ImGui::SliderFloat3(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, vec2>) {
          changed |= ImGui::SliderFloat2(name, &value.x, range.min, range.max);

        } else {
          static_assert(std::is_same_v<type_t, float>);
          changed |= ImGui::SliderFloat(name, &value, range.min, range.max);
        }

      } else if constexpr(@member_has_attribute(options_t, i, drag_float)) {
        auto drag = @member_attribute(options_t, i, drag_float);

        if constexpr(std::is_same_v<type_t, vec4>) {
          changed |= ImGui::DragFloat4(name, &value.x, drag);

        } else if constexpr(std::is_same_v<type_t, vec3>) {
          changed |= ImGui::DragFloat3(name, &value.x, drag);

        } else if constexpr(std::is_same_v<type_t, vec2>) {
          changed |= ImGui::DragFloat2(name, &value.x, drag);

        } else {
          static_assert(std::is_same_v<type_t, float>);
          changed |= ImGui::DragFloat(name, &value, drag);
        }

      } else if constexpr(@member_has_attribute(options_t, i, range_int)) {
        auto range = @member_attribute(options_t, i, range_int);

        if constexpr(std::is_same_v<type_t, ivec4>) {
          changed |= ImGui::SliderInt4(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, ivec3>) {
          changed |= ImGui::SliderInt3(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, ivec2>) {
          changed |= ImGui::SliderInt2(name, &value.x, range.min, range.max);

        } else {
          static_assert(std::is_same_v<type_t, int>);
          changed |= ImGui::SliderInt(name, &value, range.min, range.max);
        }

      } else if constexpr(std::is_same_v<type_t, bool>) {
        changed |= ImGui::Checkbox(name, &value);

      } else if constexpr(std::is_same_v<type_t, vec4>) {
        changed |= ImGui::DragFloat4(name, &value.x, .01f);

      } else if constexpr(std::is_same_v<type_t, vec3>) {
        changed |= ImGui::DragFloat3(name, &value.x, .01f);

      } else if constexpr(std::is_same_v<type_t, vec2>) {
        changed |= ImGui::DragFloat2(name, &value.x, .01f);
        
      } else if constexpr(std::is_same_v<type_t, float>) {
        changed |= ImGui::DragFloat(name, &value, .01f);
    
      } else if constexpr(std::is_same_v<type_t, int>) {
        changed |= ImGui::DragInt(name, &value);

      } else if constexpr(std::is_class_v<type_t>) {
        // Iterate over each data member.
        changed |= render_imgui(value, name);

      } else if constexpr(std::is_array_v<type_t>) {
        int i = 0;
        for(auto& item : value) {
          std::string s = name + ("[" + std::to_string(i++) + "]");
          changed |= ImGui::DragFloat(s.c_str(), &item, .01f);
        }
      }
    }}

    if(child_name)
      ImGui::TreePop();
  }

  return changed;
}

////////////////////////////////////////////////////////////////////////////////

[[using spirv: in, location(0)]]
vec4 vertex_in;

[[spirv::vert]]
void vert_main() {
  glvert_Output.Position = vertex_in;
}

[[using spirv: out, location(0)]]
vec4 fragColor;

struct shadertoy_uniforms_t {
  // shader-specific parameters.
  vec4 mouse;            // .xy is current or last drag position.
                         // .zw is current or last click.
                         // .zw is negative if mouse button is up.
  vec2 resolution;       // width and height of viewport in pixels.
  float time;            // seconds since simulation started.
};

[[using spirv: uniform, binding(0)]]
shadertoy_uniforms_t uniforms;

template<typename shader_t>
[[using spirv: uniform, binding(1)]]
shader_t shader_ubo;

template<typename shader_t>
[[spirv::frag(lower_left)]]
void frag_main() {
  fragColor = shader_ubo<shader_t>.render(glfrag_FragCoord.xy, uniforms);
}

// Load the JSON file.
#ifndef WEIGHTS_JSON_FILENAME
#define WEIGHTS_JSON_FILENAME "bunny.json"
#endif

@meta std::vector<float> weights_vector, biases_vector;

@meta+ {
  std::ifstream i(WEIGHTS_JSON_FILENAME);

  if(!i.is_open()) {
    fprintf(stderr, "cannot open scene file %s\n", WEIGHTS_JSON_FILENAME);
    exit(1);
  }
  
  nlohmann::json weights_json;
  i>> weights_json;

  // Load the weights and biases into meta vectors.
  for(auto& j2 : weights_json["weights"])
    weights_vector.push_back((float)j2.get<double>());
  for(auto& j2 : weights_json["biases"])
    biases_vector.push_back((float)j2.get<double>());
}

// Create constexpr arrays for the weights and biases.
// NOTE: Extend Circle to allow sourcing directly on std::vector.
const float weights[] = @array(weights_vector.data(), weights_vector.size());
const float biases[] = @array(biases_vector.data(), biases_vector.size());

constexpr const char* titles[] {
  "Neural SDF - Private weights",
  "Neural SDF - UBO weights"
};

template<bool use_members>
struct [[
  .imgui::title=titles[(int)use_members],
  .imgui::url="https://www.shadertoy.com/view/wdVfzz"
]] bunny_t {

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    vec2 p = (frag_coord / u.resolution) * 2 - 1;
    p.x *= u.resolution.x / u.resolution.y;

    // ray cast
    float rx = ((u.mouse.y / u.resolution.y) - 0.5f) * 3;    
    float ry = ((u.mouse.x / u.resolution.x) - 0.5f) * 6;
    
    // camera
    mat3 m = rotY(ry) * rotX(-rx);
    vec3 ro = m * vec3(0, 0, 1.5) * Zoom;
    vec3 rd = m * normalize(vec3(p, -2));
            
    bool hit=false;
    vec3 hitPos = raycast(ro, rd, hit);

    vec4 color;
    if codegen(__is_spirv_target) {
      vec3 n = normalize(cross(glfrag_dFdx(hitPos), glfrag_dFdy(hitPos)));
      color = vec4(hit ? n * .5f + .5f : 0.f, 1);

    } else {
      color = hit ? vec4(1, 0, 0, 0) : vec4(0);
    }

    return color;
  }

  float leakyReLU(float x) {
    return max(0.0f, x) + 0.1f * min(0.0f, x);
  }

  static constexpr int H = 32; // size of hidden layers

  // compute MLP network layer
  // Note array_t deduction for Circle SPIR-V depointerization bug.
  template<int N, int M, typename array_t>
  void computeLayer(array_t& I, int wi, int bi, array_t& O, bool relu) {
    if constexpr(use_members) {
      @meta for(int i=0; i<M; i++) {{
        // Compile-time loop unrolling is crucial here. Without this, the
        // program runs 100x slower.
        float r = 0.0f;
        @meta for(int j=0; j<N; j++)
          r += I[j] * weights[i*N+j+wi];
          
        r += biases[i+bi];
        O[i] = relu ? leakyReLU(r) : r;
      }}

    } else {
      @meta for(int i=0; i<M; i++) {{
        float r = 0.0f;
        @meta for(int j=0; j<N; j++)
          r += I[j] * ::weights[i*N+j+wi];
          
        r += ::biases[i+bi];
        O[i] = relu ? leakyReLU(r) : r;
      }}
    }
  }

  // evaluate network at position p
  // returns distance to nearest surface
  float network(vec3 p)
  {
      // temp storage
      float _out[H];
      float _out2[H];
      
      // input
      _out2[0] = p.x;
      _out2[1] = p.y;
      _out2[2] = p.z;
      
      int wi = 0;
      int bi = 0;
      
      // layer 0 (input)
      computeLayer<3, H>(_out2, wi, bi, _out, true);
      wi += H*3;
      bi += H;
          
      // layer 1
      computeLayer<H, H>(_out, wi, bi, _out2, true);
      wi += H*H;
      bi += H;
      
    // layer 2
      computeLayer<H, H>(_out2, wi, bi, _out, true);
      wi += H*H;
      bi += H;
      
    // layer 3
      computeLayer<H, H>(_out, wi, bi, _out2, true);
      wi += H*H;
      bi += H;
      
      // layer 4 (output)
      computeLayer<H, 1>(_out2, wi, bi, _out, true);
      
      return tanh(_out[0]);
  }

  float sphere(vec3 p, float r)
  {
      return length(p) - r;
  }

  // cast ray using sphere tracing
  vec3 raycast(vec3 ro, vec3 rd, bool& hit)
  {
      hit = false;
      vec3 pos = ro;

      for(int i=0; i<maxSteps; i++)
      {
          float d = network(pos);
          if (abs(d) < .001f * hitThreshold) {
              hit = true;
              break;
          }
          pos += d*rd;
      }
      return pos;
  }

  mat3 rotX(float a)
  {
      float sa = sin(a);
      float ca = cos(a);
      return mat3(1.0, 0.0, 0.0, 0.0, ca, -sa, 0.0, sa, ca);
  }

  mat3 rotY(float a)
  {
      float sa = sin(a);
      float ca = cos(a);
      return mat3(ca, 0.0, sa, 0.0, 1.0, 0.0, -sa, 0, ca);
  }

  [[.imgui::range_int {1, 64}]] int maxSteps = 20;
  [[.imgui::drag_float=.01]] float hitThreshold = 1;
  [[.imgui::range_float { .1, 3 }]] float Zoom = 1;

  @meta if(use_members) {
    float weights[::weights.length] = { @pack_nontype(::weights)... };
    float biases[::biases.length] = { @pack_nontype(::biases)... }; 
  }
};

enum typename class shader_program_t {
  bunny_t<false>,
  bunny_t<true>,
};


////////////////////////////////////////////////////////////////////////////////

struct program_base_t {
  // Return true if any parameter has changed.
  virtual bool configure(bool update_ubo) = 0;

  // Evaluate the shader with the CPU at this coordinate.
  virtual vec4 eval(vec2 coord, shadertoy_uniforms_t u, 
    bool signal = false) = 0;

  GLuint program;
  GLuint ubo;
};

template<typename shader_t>
struct program_t : program_base_t {
  // Keep an instance of the shader parameters in memory to drive ImGui.
  shader_t shader;

  program_t();
  bool configure(bool update_ubo) override;
  vec4 eval(vec2 coord, shadertoy_uniforms_t u, bool signal) override;
};

template<typename shader_t>
program_t<shader_t>::program_t() {
  // Create vertex and fragment shader handles.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, fs };

  // Associate shader handlers with the translation unit's SPIRV data.
  glShaderBinary(2, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, 
    __spirv_data, __spirv_size);
  glSpecializeShader(vs, @spirv(vert_main), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_main<shader_t>), 0, nullptr, nullptr);

  // Link the shaders into a program.
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // Create the UBO.
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(shader_t), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);
}

template<typename shader_t>
bool program_t<shader_t>::configure(bool update_ubo) {
  bool changed = render_imgui(shader);
  if(update_ubo)
    glNamedBufferSubData(ubo, 0, sizeof(shader_t), &shader);

  return changed;
}

template<typename shader_t>
vec4 program_t<shader_t>::eval(vec2 coord, shadertoy_uniforms_t u, 
  bool signal) {

  if(signal)
    raise(SIGINT);

  return shader.render(coord, u);
}

////////////////////////////////////////////////////////////////////////////////

struct software_fbo_t {
  software_fbo_t(int width, int height);
  ~software_fbo_t();

  // Maintain an FBO and texture for running the shader program on the CPU.
  // We'll then upload it to this texture and glNamedFramebufferBlit it.
  GLuint texture;
  GLuint fbo;
  int width, height;

  std::vector<uint32_t> data;

  void update();
  void blit(int width2, int height2);
  void set_block(vec4 color, int x, int y, int sx, int sy);
};

software_fbo_t::software_fbo_t(int width, int height) :
  width(width), height(height) {

  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glCreateFramebuffers(1, &fbo);

  glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texture, 0);

  data.resize(width * height);
}

software_fbo_t::~software_fbo_t() {
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &texture);
}

void software_fbo_t::update() {
  // Upload the texture data.
  glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, 
    GL_UNSIGNED_BYTE, data.data());
}

void software_fbo_t::blit(int width2, int height2) {
  // Get the attached renderbuffer.
  GLint fbo_dest;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo_dest);

  // Blit the texture to the renderbuffer.
  glBlitNamedFramebuffer(fbo, fbo_dest, 0, 0, width2, height2,
    0, 0, width2, height2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void software_fbo_t::set_block(vec4 color, int x, int y, int sx, int sy) {
  // Convert the color to RGBA8.
  ivec4 color2 = clamp((ivec4)(255.f * color + 1.f / 512), 0, 255);
  uint32_t packed = color2.r | (color2.g<< 8) | (color2.b<< 16) | 0xff000000;

  int index = width * y + x;
  for(int row = 0; row < sy; ++row) {
    for(int col = 0; col < sx; ++col)
      data[index + col] = packed;
    index += width;
  }
}

struct cpu_compute_t {
  cpu_compute_t(int num_threads, int width, int height);
  ~cpu_compute_t();

  void pool_execute();
  void thread_execute(int tid);
  bool pixel_execute(int x, int y, int sx, int sy);
  bool is_complete() const;
  void join();
  void update();

  static void thread_entry(cpu_compute_t* compute, int tid);

  std::vector<std::thread> threads;
  std::atomic<int> complete_count;
  std::atomic<bool> okay;

  int width, height;
  shadertoy_uniforms_t uniforms;
  int num_levels = 0;
  bool interlace = false;

  program_base_t* program;
  std::unique_ptr<software_fbo_t> fbo;
};

cpu_compute_t::cpu_compute_t(int num_threads, int width, int height) :
  width(width), height(height) {

  complete_count = num_threads;
  threads.resize(num_threads);
  okay = false;

  int width2 = (width + 7) & ~7;
  int height2 = (height + 7) & ~7;

  fbo = std::make_unique<software_fbo_t>(width2, height2);
}

cpu_compute_t::~cpu_compute_t() {
  join();
}

void cpu_compute_t::pool_execute() {
  okay = true;
  complete_count = 0;
  for(int tid = 0; tid < threads.size(); ++tid)
    threads[tid] = std::thread(thread_entry, this, tid);
}

void cpu_compute_t::thread_execute(int tid) {
  adam7_t adam7 { (width + 7) / 8, (height + 7) / 8 };

  auto f = [&](int x, int y, int sx, int sy) {
    return pixel_execute(x, y, sx, sy);
  };
  adam7.process(tid, threads.size(), num_levels, interlace, f);

  ++complete_count;
}

bool cpu_compute_t::pixel_execute(int x, int y, int sx, int sy) {
  // Immediately break if any setting has changed.
  if(!okay)
    return false;

  // Adjust to get the center of the pixel.
  float x2 = x + .5f;
  float y2 = y + .5f;

  vec4 color = program->eval(vec2(x2, y2), uniforms);
  fbo->set_block(color, x, y, sx, sy);

  return true;
}

bool cpu_compute_t::is_complete() const {
  return threads.size() == complete_count;
}

void cpu_compute_t::join() {
  okay = false;
  for(std::thread& t : threads)
    if(t.joinable())
      t.join();
}

void cpu_compute_t::thread_entry(cpu_compute_t* compute, int tid) {
  compute->thread_execute(tid);
}


////////////////////////////////////////////////////////////////////////////////

struct app_t {
  GLFWwindow* window = nullptr;
  GLuint vao = 0;
  GLuint array_buffer = 0;
  GLuint uniform_buffer = 0;

  // Indicate the current program being rendered.
  shader_program_t active_shader = { };

  shadertoy_uniforms_t uniforms;

  // Keep an instance of the current program.
  std::unique_ptr<program_base_t> program;

  float speed = 1;
  double time = 0;
  bool debug_on_click = false;
  bool render_cpu = false;
  bool interlace = false;
  bool asynchronous = true;
  int num_threads = 1;
  int num_levels = 1;
  std::unique_ptr<software_fbo_t> software_fbo;
  std::unique_ptr<cpu_compute_t> cpu_compute;

  app_t();
  void loop();
  void button_callback(int button, int action, int mods);

  bool configure();
  void set_active_shader(shader_program_t shader);

  void display_gpu();
  void display_cpu();

  // Fully compute one thread's contribution to the shader program.
  bool compute_cpu(int tid, int num_threads, int block_size);

  void framebuffer_callback(int width, int height);
  static void _framebuffer_callback(GLFWwindow* window, int width, int height);
  static void _button_callback(GLFWwindow* window, int button, int action, 
    int mods);

  static void _debug_callback(GLenum source, GLenum type, GLuint id, 
    GLenum severity, GLsizei length, const GLchar* message, 
    const void* user_param);
};

void app_t::framebuffer_callback(int width, int height) {
  glViewport(0, 0, width, height);
  cpu_compute.reset();
}

void app_t::_framebuffer_callback(GLFWwindow* window, int width, int height) {
  app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
  app->framebuffer_callback(width, height);
}

void app_t::_button_callback(GLFWwindow* window, int button, int action,
  int mods) {
  app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
  app->button_callback(button, action, mods);
}

void app_t::_debug_callback(GLenum source, GLenum type, GLuint id, 
  GLenum severity, GLsizei length, const GLchar* message, 
  const void* user_param) {

  if(GL_DEBUG_SEVERITY_HIGH == severity ||
    GL_DEBUG_SEVERITY_MEDIUM == severity)
    printf("OpenGL: %s\n", message);

  if(GL_DEBUG_SEVERITY_HIGH == severity)
    exit(1);
}

app_t::app_t() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

  window = glfwCreateWindow(1280, 720, "Circle does Shadertoy", 
    NULL, NULL);
  if(!window) {
    printf("Cannot create GLFW window\n");
    exit(1);
  }

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(_debug_callback, this);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, _framebuffer_callback);
  glfwSetMouseButtonCallback(window, _button_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Create an ImGui context.
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  const float vertices[][2] { { -1, 1 }, { 1, 1 }, { -1, -1 }, { 1, -1 } };

  // Load into an array object.
  glCreateBuffers(1, &array_buffer);
  glNamedBufferStorage(array_buffer, sizeof(vertices), vertices, 0);

  // Declare a vertex array object and bind the array buffer.
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer);

  // Bind to slot 0
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // Allocate a shadertoy_uniform_t buffer.
  glCreateBuffers(1, &uniform_buffer);
  glNamedBufferStorage(uniform_buffer, sizeof(shadertoy_uniforms_t), nullptr,
    GL_DYNAMIC_STORAGE_BIT);

  // Choose the default shader.
  set_active_shader(shader_program_t { });

  uniforms.time = 0;
  uniforms.mouse = vec4(.5, .5, .5, .5);
}

void app_t::loop() {

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Set the shadertoy uniforms.
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Let ImGui have the mouse if the cursor is above its windows.
    ImGuiIO& io = ImGui::GetIO();
    bool has_mouse = !io.WantCaptureMouse &&
      GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if(has_mouse) {
      // Update mouse constant if pressed.
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);

      float x = floor(xpos) + .5f;
      float y = height - floor(ypos) - .5f;

      uniforms.mouse.x = x;
      uniforms.mouse.y = y;
      if(uniforms.mouse.z < 0) {
        // The button was up the previous frame.
        uniforms.mouse.z = x;
        uniforms.mouse.w = y;
      }

    } else if(uniforms.mouse.z >= 0) {
      // The button was down the previous frame.
      // uniforms.mouse.zw *= -uniforms.mouse.xy;
      uniforms.mouse.z = -uniforms.mouse.x;
      uniforms.mouse.w = -uniforms.mouse.y;
    }

    uniforms.resolution = vec2(width, height);
    double new_time = glfwGetTime();
    uniforms.time += speed * (new_time - time);
    time = new_time; 

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Configure the selected program...
    bool changed = configure();

    // Save the ImGui frame.
    ImGui::Render();

    if(render_cpu) {
      if(!cpu_compute || cpu_compute->threads.size() != num_threads) {
        // Create a new thread pool.
        cpu_compute = std::make_unique<cpu_compute_t>(num_threads, width,
          height);
        cpu_compute->program = program.get();
      }

      if(changed || cpu_compute->is_complete()) {
        // Either the shader settings have changed or we finished the frame.
        // Either way, update the settings and start again.
        cpu_compute->join();

        cpu_compute->fbo->update();
        cpu_compute->fbo->blit(cpu_compute->width, cpu_compute->height);

        cpu_compute->num_levels = num_levels;
        cpu_compute->interlace = interlace;
        cpu_compute->uniforms = uniforms;
        cpu_compute->pool_execute();

      } else if(asynchronous)
        cpu_compute->fbo->update();

      // Render what we have so far.
      cpu_compute->fbo->blit(cpu_compute->width, cpu_compute->height);

    } else {
      if(cpu_compute) {
        cpu_compute.reset();
      }

      // Bind the builtin shadertoy uniforms.
      glNamedBufferSubData(uniform_buffer, 0, sizeof(uniforms), &uniforms);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);

      // Bind and execute the input program.
      glUseProgram(program->program);
      glBindBufferBase(GL_UNIFORM_BUFFER, 1, program->ubo);

      glBindVertexArray(vao);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Render the ImGui frame over the application.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers.
    glfwSwapBuffers(window);
  }
}

void app_t::button_callback(int button, int action, int mods) {
  ImGuiIO& io = ImGui::GetIO();
  if(!io.WantCaptureMouse && GLFW_PRESS == action && 
    GLFW_MOUSE_BUTTON_LEFT == button) {

    // On the CPU, run the shader once on the click-on point.
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    vec2 coord((float)x + .5f, uniforms.resolution.y - (float)y - .5f);
    shadertoy_uniforms_t u = uniforms;
    u.mouse = vec4(coord, coord);

    vec4 color = program->eval(coord, u, debug_on_click);

    printf("(%f, %f) -> (%f %f %f %f)\n", coord.x, coord.y,
      color.r, color.g, color.b, color.a);
  }
}

bool app_t::configure() {
  // Start by selecting the current program.
  // Produce a combo box with all shader options.
  ImGui::Begin("Shader parameters");

  ImGui::DragFloat("Time", &uniforms.time, .1);
  ImGui::SliderFloat("Speed", &speed, 0, 5);

  ImGui::Checkbox("Debug on click (run from GDB)", &debug_on_click);

  ImGui::Checkbox("Render on CPU", &render_cpu);

  bool changed = false;
  if(render_cpu) {
    changed |= ImGui::SliderInt("Thread pool size", &num_threads, 1, 32);
    changed |= ImGui::SliderInt("Detail levels", &num_levels, 1, 7);
    changed |= ImGui::Checkbox("Interlacing", &interlace);
    ImGui::Checkbox("Asynchronous", &asynchronous);
  }

  int current = (int)active_shader;
  const char* items[] { 
    @attribute(@enum_types(shader_program_t), imgui::title)... 
  }; 
  changed |= ImGui::Combo("Active shader", &current, items, items.length);

  if(current != (int)active_shader) {
    cpu_compute.reset();
    set_active_shader((shader_program_t)current);
  }

  program->configure(!render_cpu);
  ImGui::End();

  return changed;
}

void app_t::set_active_shader(shader_program_t shader) {
  // Generate a switch construct the requested shader program.
  switch(shader) {
    @meta for enum(shader_program_t e : shader_program_t) {
      case e: 
        program = std::make_unique<program_t<@enum_type(e)> >();
        glfwSetWindowTitle(window, @attribute(@enum_type(e), imgui::title));
        break;
    }
  }
  active_shader = shader;
}


////////////////////////////////////////////////////////////////////////////////


int main() {
  

  glfwInit();
  gl3wInit();
  
  app_t app;
  app.loop();

  return 0;
}
