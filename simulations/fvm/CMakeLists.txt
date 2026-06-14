if(Kokkos_FOUND)
  file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

  get_filename_component(SIM_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  add_executable(${SIM_NAME} ${sources})
  target_include_directories(${SIM_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${SIM_NAME} PRIVATE flim flimkokkos)
endif()
