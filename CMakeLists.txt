project(MilkdropShaderConverter)
cmake_minimum_required(VERSION 3.1)

add_subdirectory(glsl-optimizer EXCLUDE_FROM_ALL)
add_subdirectory(hlsl2glslfork EXCLUDE_FROM_ALL)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(MilkdropShaderConverter src/main.cpp)
target_include_directories(MilkdropShaderConverter PRIVATE
  ${OPENGL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MilkdropShaderConverter
  ${OPENGL_LIBRARIES} Threads::Threads hlsl2glsl glsl_optimizer)
