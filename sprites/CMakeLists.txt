cmake_minimum_required(VERSION 3.9)
project(sprites)

include_directories(../include)

set(SOURCE_FILES
  sprites.cxx
)

set_source_files_properties(sprites.cxx PROPERTIES COMPILE_FLAGS -shader)

add_executable(sprites ${SOURCE_FILES})

target_link_libraries(sprites
  glfw
  gl3w
  GL
)
