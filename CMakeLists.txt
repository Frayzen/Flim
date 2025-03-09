cmake_minimum_required(VERSION 3.10)
project(Flim)


find_package(HIP QUIET)
if(HIP_FOUND)
    message(STATUS "Found HIP: " ${HIP_VERSION})
else()
    message(FATAL_ERROR "Could not find HIP. Ensure that HIP is either installed in /opt/rocm/hip or the variable HIP_PATH is set to point to the right location.")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic -std=c++20")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
endif()

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  find_package(VulkanValidationLayers REQUIRED)
endif(CMAKE_BUILD_TYPE EQUAL "DEBUG")

# Kokkos related
find_package(Kokkos REQUIRED)

file(GLOB_RECURSE sources src/*.cc)
file(GLOB_RECURSE headers src/*.hh)
add_library(flim ${sources})

target_link_libraries(flim ${Vulkan_LIBRARY} glfw Eigen3::Eigen assimp imgui Kokkos::kokkos)
target_include_directories(flim PUBLIC ${CMAKE_SOURCE_DIR}/src)

find_package(HIP QUIET)
if(HIP_FOUND)
    message(STATUS "Found HIP: " ${HIP_VERSION})
    target_link_libraries(flim hip::device hip::host)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I$ENV{ROCM_PATH}/include")
else()
    message(FATAL_ERROR "Could not find HIP. Ensure that HIP is either installed in /opt/rocm/hip or the variable HIP_PATH is set to point to the right location.")
endif()


add_subdirectory(simulations)
add_subdirectory(shaders)

