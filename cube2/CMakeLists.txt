cmake_minimum_required(VERSION 3.9)
project(cube2)

include_directories(../include)

set(SOURCE_FILES
  cube2.cxx
)

set_source_files_properties(cube2.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(cube2 ${SOURCE_FILES})

target_link_libraries(cube2
  glfw
  gl3w
  GL
)
