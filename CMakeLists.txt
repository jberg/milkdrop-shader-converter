project(MilkdropShaderConverter)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (UNIX)
  add_definitions(-Dlinux)
endif ()

add_subdirectory(glsl-optimizer EXCLUDE_FROM_ALL)
add_subdirectory(hlsl2glslfork EXCLUDE_FROM_ALL)

add_library(MilkdropShaderConverter SHARED src/main.cpp)
target_include_directories(MilkdropShaderConverter PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MilkdropShaderConverter
  hlsl2glsl glsl_optimizer)

set_target_properties(MilkdropShaderConverter PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(MilkdropShaderConverter PRIVATE ${CMAKE_JS_INC})
target_link_libraries(MilkdropShaderConverter ${CMAKE_JS_LIB})
