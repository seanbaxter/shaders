cmake_minimum_required(VERSION 3.9)

project(shadertoy3)

include_directories(../thirdparty/imgui)

set(SOURCE_FILES
  shadertoy3.cxx

  ../thirdparty/imgui/imgui.cpp
  ../thirdparty/imgui/imgui_draw.cpp
  ../thirdparty/imgui/imgui_widgets.cpp
  ../thirdparty/imgui/backends/imgui_impl_glfw.cpp
  ../thirdparty/imgui/backends/imgui_impl_opengl3.cpp  
)

add_compile_options(
  "$<$<CONFIG:DEBUG>:-g>"
)

set_source_files_properties(shadertoy3.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(shadertoy3 ${SOURCE_FILES})

target_link_libraries(shadertoy3
  glfw
  gl3w
  GL
)