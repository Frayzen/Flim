cmake_minimum_required(VERSION 3.5)
project(engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -std=c++20 -Wno-old-style-cast")

file(GLOB_RECURSE sources src/*.cc)
add_executable(engine ${sources})

