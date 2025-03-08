cmake_minimum_required(VERSION 3.10)
project(Flim)

include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic -std=c++20")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
endif()

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  find_package(VulkanValidationLayers REQUIRED)
endif(CMAKE_BUILD_TYPE EQUAL "DEBUG")

file(GLOB_RECURSE sources src/*.cc)
file(GLOB_RECURSE headers src/*.hh)
add_library(flim ${sources})

target_link_libraries(flim PUBLIC ${Vulkan_LIBRARY} glfw Eigen3::Eigen assimp imgui)
target_include_directories(flim PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(simulations)
add_subdirectory(shaders)
