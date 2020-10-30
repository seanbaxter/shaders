#define STBI_ONLY_PNG
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb/stb_image.h"
#include <string>
#include <vector>
#include <gl3w/GL/gl3w.h>
#include <GLFW/glfw3.h>

struct sprite_sheet_t {
  sprite_sheet_t(const char* metadata, const char* image);
  ~sprite_sheet_t();

  struct sprite_t {
    std::string name;
    int left, top, width, height;
  };
  std::vector<sprite_t> sprites;
  uint32_t transparent;

  const uint32_t* data = nullptr;
  int width, height;
};

inline sprite_sheet_t::sprite_sheet_t(const char* metadata, const char* image) {
  FILE* f = fopen(metadata, "r");
  if(!f) {
    printf("Cannot load sprite sheet metadata %s\n", metadata);
    exit(1);
  }

  // The first three numbers in the sprite sheet are rgb components of the
  // transparency color.
  int r, g, b;
  fscanf(f, "%d %d %d", &r, &g, &b);

  // Get a transparent pixel value.
  transparent = r | (g<< 8) | (b<< 16) | (255<< 24);

  // Load in each row of the sprite sheet.
  char name[64];
  int left, top, right, bottom;
  while(5 == fscanf(f, "%s %d %d %d %d", name, &left, &top, &right, &bottom)) {
    sprites.push_back({
      name, left, top, right - left, bottom - top
    });
  }
  fclose(f);

  // Open the image with RGBA format.
  int comp;
  data = (uint32_t*)stbi_load(image, &width, &height, &comp, STBI_rgb_alpha);

  if(!data) {
    printf("Cannot load image data %s\n", image);
    exit(1);
  }
}

inline sprite_sheet_t::~sprite_sheet_t() {
  if(data)
    stbi_image_free((void*)data);
}

@meta sprite_sheet_t sprite_sheet(
  "../assets/tyrian.sprites", 
  "../assets/tyrian.png"
);

const int NumSprites = sprite_sheet.sprites.size();

// Generate an enum with a name for each sprite.
enum class sprite_name_t {
  @meta for(const auto& sprite : sprite_sheet.sprites)
    @(sprite.name);
};

// Print the loaded sprites.
@meta printf("* %s\n", @enum_names(sprite_name_t))...;

////////////////////////////////////////////////////////////////////////////////

// Treat the rgba8ui image as an r32ui image for more efficient imageStore.
[[using spirv: uniform, binding(0), format(r32ui)]]
uimage2D output_image;

[[using spirv: buffer, binding(0)]]
ivec2 sprite_locations[];

[[using spirv: uniform, location(0)]]
int sprite_count;

template<sprite_name_t name> 
[[using spirv: comp, local_size(32)]]
void comp_sprite() {
  uint gid = glcomp_GlobalInvocationID.x;
  if(gid >= sprite_count)
    return;

  ivec2 location = sprite_locations[gid];

  @meta+ {
    printf("Generating compiled sprite shader '%s'\n", @enum_name(name));

    // Find the offset into the sprite PNG data.
    auto sprite = sprite_sheet.sprites[(int)name];
    const uint32_t* row_data = sprite_sheet.data + 
      sprite.top * sprite_sheet.width + sprite.left;

    for(int row = 0; row < sprite.height; ++row) {
      for(int col = 0; col < sprite.width; ++col) {
        // Emit an imageStore to write the compiled sprite.
        if(sprite_sheet.transparent != row_data[col]) {
          @emit imageStore(
            output_image, 
            location + ivec2(col, row), 
            uvec4(row_data[col])
          );
        }
      }

      row_data += sprite_sheet.width;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

template<typename type_t>
const char* enum_to_string(type_t e) {
  static_assert(std::is_enum_v<type_t>);
  switch(e) {
    @meta for enum(type_t e2 : type_t) {
      case e2:
        return @enum_name(e2);
    }
    default:
      return nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////

// Establish a size for the game world.
// Each logical pixel gets blown up to 4x4 screen pixels. This looks retro.
const int PixSize = 4;
const int Width = 800 / PixSize;
const int Height = 720 / PixSize;

struct app_t {
  app_t();
  void loop();
  void display();

  void draw_sprite(int x, int y, sprite_name_t name);
  void button_callback(int button, int action, int mods);
  void scroll_callback(double x, double y);
  void key_callback(int key, int scancode, int action, int mods);

  enum { MaxSprites = 1024 };
  
  GLFWwindow* window;

  // The current sprite selected. Insert a new one of these whenever the mouse
  // is clicked.
  sprite_name_t cur_sprite { };
  double last_time = 0;

  GLuint tex;         // Texture backing for offscreen fbo.
  GLuint fbo;         // An offscreen framebuffer.

  GLuint compute_programs[NumSprites];

  std::vector<ivec2> sprite_locations;
  GLuint sprite_locations_buffer;
};

void debug_callback(GLenum source, GLenum type, GLuint id, 
  GLenum severity, GLsizei length, const GLchar* message, 
  const void* user_param) { 

  if(GL_DEBUG_SEVERITY_HIGH == severity ||
    GL_DEBUG_SEVERITY_MEDIUM == severity)
    printf("OpenGL: %s\n", message);

  if(GL_DEBUG_SEVERITY_HIGH == severity)
    exit(1);
}

app_t::app_t() {
  // Construct the GLFW window.
  glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  window = glfwCreateWindow(1280, 720, 
    "compiled sprites - use mousewheel or arrow keys to change ammo",
     nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Set mouse and scrollwheel callbacks.
  auto bc = [](GLFWwindow* window, int button, int action, int mods) {
    app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
    app->button_callback(button, action, mods);
  };
  glfwSetMouseButtonCallback(window, bc);

  auto kc = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
    app->key_callback(key, scancode, action, mods);
  };
  glfwSetKeyCallback(window, kc);

  auto sc = [](GLFWwindow* window, double x, double y) {
    app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
    app->scroll_callback(x, y);
  };
  glfwSetScrollCallback(window, sc);

  // Set debug message callbacks.
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debug_callback, nullptr);

  // Allocate a compute shader for each sprite.
  GLuint compute_shaders[NumSprites];
  compute_shaders[:] = glCreateShader(GL_COMPUTE_SHADER)...;

  // Associate each shader with the TU's SPIR-V module.
  glShaderBinary(NumSprites, compute_shaders, GL_SHADER_BINARY_FORMAT_SPIR_V,
    __spirv_data, __spirv_size);

  // Specialize a compute shader for each sprite.
  glSpecializeShader(
    compute_shaders...[:], 
    @spirv(comp_sprite<@enum_values(sprite_name_t)>), 
    0, 
    nullptr, 
    nullptr
  )...;

  // Create a program for each shader.
  compute_programs[:] = glCreateProgram()...;
  glAttachShader(compute_programs[:], compute_shaders[:])...;
  glLinkProgram(compute_programs[:])...;

  GLint info[NumSprites];
  glGetProgramiv(compute_programs[:], GL_LINK_STATUS, &info[:])...;

  // Create memory backing for the offscreen image.
  glCreateTextures(GL_TEXTURE_2D, 1, &tex);
  glTextureStorage2D(tex, 1, GL_RGBA8, Width, Height);
  
  glCreateFramebuffers(1, &fbo);
  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, tex, 0);

  // Create buffer to hold sprite locations.
  glCreateBuffers(1, &sprite_locations_buffer);
  glNamedBufferStorage(sprite_locations_buffer, sizeof(ivec2) * MaxSprites, 
    nullptr, GL_DYNAMIC_STORAGE_BIT);
}

void app_t::loop() {
  while(!glfwWindowShouldClose(window)) {
    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}


void app_t::button_callback(int button, int action, int mods) {
  if(GLFW_PRESS == action && GLFW_MOUSE_BUTTON_LEFT == button) {
    double x_, y_;
    glfwGetCursorPos(window, &x_, &y_);

    // Find the corner of the image.
    int width2, height2;
    glfwGetWindowSize(window, &width2, &height2);

    x_ -= (width2 - PixSize * Width) / 2;
    y_ -= (height2 - PixSize * Height) / 2;

    // Center the sprite.
    int x = (int)x_ / PixSize - 6;
    int y = (int)y_ / PixSize - 7;

    sprite_locations.push_back({ x, y });
  }
}

void app_t::scroll_callback(double x, double y) {
  int y2 = (int)y;
  int next = ((int)cur_sprite + y2) % @enum_count(sprite_name_t);
  cur_sprite = (sprite_name_t)next;

  char title[128];
  snprintf(title, 128, "[[ %s ]]", enum_to_string(cur_sprite));
  glfwSetWindowTitle(window, title);
}

void app_t::key_callback(int key, int scancode, int action, int mods) {
  int diff = 0;
  if(GLFW_PRESS == action) {
    if(GLFW_KEY_LEFT == key) diff = -1;
    else if(GLFW_KEY_RIGHT == key) diff = 1;  
  }

  if(diff) {
    int next = ((int)cur_sprite + diff) % @enum_count(sprite_name_t);
    cur_sprite = (sprite_name_t)next;

    char title[128];
    snprintf(title, 128, "[[ %s ]]", enum_to_string(cur_sprite));
    glfwSetWindowTitle(window, title);
  }
}

void app_t::display() {
  // Clear screen to black.
  const float bg[4] { };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  // Animate the background.
  std::vector<uint32_t> framebuffer;
  framebuffer.resize(Width * Height);
  for(int row = 0; row < Height; ++row) {
    for(int col = 0; col < Width; ++col)
      framebuffer[row * Width + col] = row;
  }

  // Copy the software buffer to the offscreen buffer.
  glTextureSubImage2D(tex, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE,
    framebuffer.data());

  // Advect the sprites 100 pixels/second.
  double time = glfwGetTime();
  double elapsed = time - last_time;
  last_time = time;
  int advect = (int)(100 * elapsed);

  // Change sprite locations.
  for(int i = 0; i < sprite_locations.size(); ) {
    ivec2& item = sprite_locations[i];
    item.y -= advect;
    if(item.y < 0) {
      std::swap(sprite_locations.back(), item);
      sprite_locations.resize(sprite_locations.size() - 1);
    } else
      ++i;
  }

  // Bind the sprite location buffer.
  if(sprite_locations.size()) {

    // Upload the sprites to the buffer.
    glNamedBufferSubData(sprite_locations_buffer, 0, 
      std::min<int>(MaxSprites, sprite_locations.size()) * sizeof(ivec2), 
      sprite_locations.data());

    // Run the compute shader to render the sprites.
    glUseProgram(compute_programs[(int)cur_sprite]);

    // Bind the framebuffer output image.
    glBindImageTexture(0, tex, 0, false, 0, GL_WRITE_ONLY, GL_R32UI);

    glUniform1i(0, sprite_locations.size());

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sprite_locations_buffer,
      0, std::min<int>(MaxSprites, sprite_locations.size())  * sizeof(ivec2));

    glDispatchCompute(1, 1, 1);

    // Unselect
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindTextureUnit(0, 0);
  }

  // Blit to the framebuffer.
  GLint fbo_dest;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo_dest);

  int width2, height2;
  glfwGetWindowSize(window, &width2, &height2);

  int x = (width2 - Width * PixSize) / 2;
  int y = (height2 - Height * PixSize) / 2;

  glBlitNamedFramebuffer(fbo, fbo_dest, 0, Height, Width, 0,
    x, y, x + PixSize * Width, y + PixSize * Height, GL_COLOR_BUFFER_BIT, 
    GL_NEAREST);
}

int main() {
  glfwInit();
  gl3wInit();

  app_t app;
  app.loop();

  return 0;
}
