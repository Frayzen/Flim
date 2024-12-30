cmake_minimum_required(VERSION 3.10)
project(engine)

include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -pedantic -std=c++20 -Wno-old-style-cast")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
endif()

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  find_package(VulkanValidationLayers REQUIRED)
endif(CMAKE_BUILD_TYPE EQUAL "DEBUG")

file(GLOB_RECURSE sources src/*.cc)
add_executable(engine ${sources})

target_link_libraries(engine ${Vulkan_LIBRARY} glfw glm::glm assimp imgui Kokkos::kokkos)
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(shaders)
