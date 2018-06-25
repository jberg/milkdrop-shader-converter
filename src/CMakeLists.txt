project(MilkdropShaderConverter)
cmake_minimum_required(VERSION 3.1)

add_subdirectory(../glsl-optimizer ./glsl-optimizer)
add_subdirectory(../hlsl2glslfork ./hlsl2glslfork)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

add_executable(MilkdropShaderConverter main.cpp)
target_link_libraries(MilkdropShaderConverter
  ${OPENGL_LIBRARIES} Threads::Threads hlsl2glsl glsl_optimizer)
