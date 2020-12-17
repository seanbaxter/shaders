cmake_minimum_required(VERSION 3.9)
project(particles)

include_directories(../../inc)

set(SOURCE_FILES
  particles.cxx
)

set_source_files_properties(particles.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(particles ${SOURCE_FILES})

target_link_libraries(particles
  glfw
  gl3w
  GL
)
