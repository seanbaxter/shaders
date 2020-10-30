cmake_minimum_required(VERSION 3.9)
project(nbody)

include_directories(../thirdparty/imgui)
include_directories(../include)

set(SOURCE_FILES
  nbody.cxx

  ../thirdparty/imgui/imgui.cpp
  ../thirdparty/imgui/imgui_draw.cpp
  ../thirdparty/imgui/imgui_widgets.cpp
  ../thirdparty/imgui/examples/imgui_impl_glfw.cpp
  ../thirdparty/imgui/examples/imgui_impl_opengl3.cpp  
)

set_source_files_properties(nbody.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(nbody ${SOURCE_FILES})

target_link_libraries(nbody
  glfw
  gl3w
  GL
)
