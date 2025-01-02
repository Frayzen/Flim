file(GLOB_RECURSE sources ${PROJECT_SOURCE_DIR}/simulations/*.cc)

add_executable(engine ${sources})
target_link_libraries(engine PRIVATE flim)
