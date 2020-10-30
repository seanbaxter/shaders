cmake_minimum_required(VERSION 3.9)

project(teapot)

include_directories(../thirdparty/imgui ../include)

set(SOURCE_FILES
  teapot.cxx

  ../thirdparty/imgui/imgui.cpp
  ../thirdparty/imgui/imgui_draw.cpp
  ../thirdparty/imgui/imgui_widgets.cpp
  ../thirdparty/imgui/backends/imgui_impl_glfw.cpp
  ../thirdparty/imgui/backends/imgui_impl_opengl3.cpp  
)

set_source_files_properties(teapot.cxx PROPERTIES COMPILE_FLAGS "-shader")

add_executable(teapot ${SOURCE_FILES})

target_link_libraries(teapot
  glfw
  gl3w
  GL
)