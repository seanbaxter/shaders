cmake_minimum_required(VERSION 3.9)
project(particles-cuda)

include_directories(../thirdparty/thrust ../thirdparty/cub ../mgpu-shaders/inc ../thirdparty/imgui)

set(SOURCE_FILES
  particles-cuda.cxx

  ../thirdparty/imgui/imgui.cpp
  ../thirdparty/imgui/imgui_draw.cpp
  ../thirdparty/imgui/imgui_widgets.cpp
  ../thirdparty/imgui/backends/imgui_impl_glfw.cpp
  ../thirdparty/imgui/backends/imgui_impl_opengl3.cpp  
)

set_source_files_properties(particles-cuda.cxx PROPERTIES COMPILE_FLAGS "-shader -cuda-path=/opt/nvidia/hpc_sdk/Linux_x86_64/21.3/cuda/11.2/ -sm_52 -sm_61 -sm_75 -g")

add_executable(particles-cuda ${SOURCE_FILES})

add_link_options("-no-demangle")
target_link_libraries(particles-cuda
  glfw
  gl3w
  GL
  /opt/nvidia/hpc_sdk/Linux_x86_64/21.3/cuda/11.2/lib64/libcudart.so
)
