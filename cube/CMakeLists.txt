cmake_minimum_required(VERSION 3.9)
project(cube)

include_directories(../include)

set(SOURCE_FILES
  cube.cxx
)

set_source_files_properties(cube.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(cube ${SOURCE_FILES})

target_link_libraries(cube
  glfw
  gl3w
  GL
)
