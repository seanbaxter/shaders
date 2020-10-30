cmake_minimum_required(VERSION 3.9)

project(viewer)

include_directories(../include)

set(SOURCE_FILES
  viewer.cxx
)

set_source_files_properties(viewer.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(viewer ${SOURCE_FILES})

target_link_libraries(viewer
  glfw
  gl3w
  GL
)
