cmake_minimum_required(VERSION 3.9)
project(geom)

include_directories(../include)

set(SOURCE_FILES
  geom.cxx
)

set_source_files_properties(geom.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(geom ${SOURCE_FILES})

target_link_libraries(geom
  glfw
  gl3w
  GL
)
