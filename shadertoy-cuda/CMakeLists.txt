cmake_minimum_required(VERSION 3.9)

project(shadertoy-cuda)

include_directories(../thirdparty/imgui)

set(SOURCE_FILES
  shadertoy.cxx

  ../thirdparty/imgui/imgui.cpp
  ../thirdparty/imgui/imgui_draw.cpp
  ../thirdparty/imgui/imgui_widgets.cpp
  ../thirdparty/imgui/backends/imgui_impl_glfw.cpp
  ../thirdparty/imgui/backends/imgui_impl_opengl3.cpp  
)

set_source_files_properties(shadertoy.cxx PROPERTIES COMPILE_OPTIONS "-g;-shader;--cuda-path=/usr/local/cuda/;-sm_75")

add_executable(shadertoy-cuda ${SOURCE_FILES})

target_link_libraries(shadertoy-cuda
  glfw
  gl3w
  GL
  /usr/local/cuda/targets/x86_64-linux/lib/libcudart.so
)