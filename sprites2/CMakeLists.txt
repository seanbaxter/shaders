cmake_minimum_required(VERSION 3.9)
project(sprites2)

include_directories(../include)

set(SOURCE_FILES
  sprites2.cxx
)

set_source_files_properties(sprites2.cxx PROPERTIES COMPILE_FLAGS "-shader -M ../thirdparty/libstbi.so")

add_executable(sprites2 ${SOURCE_FILES})

target_link_libraries(sprites2
  glfw
  gl3w
  GL
)
