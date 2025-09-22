function(InstallDeps)
  set(imgui_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/imgui)
  if(NOT EXISTS ${imgui_SOURCE_DIR})
    execute_process(COMMAND git clone https://github.com/ocornut/imgui -b
                            docking --depth 1 ${imgui_SOURCE_DIR})
  endif()

  # IMGUI TARGET
  file(GLOB imgui_sources ${imgui_SOURCE_DIR}/*.cpp)
  file(GLOB imgui_misc_sources ${imgui_SOURCE_DIR}/misc/cpp/*.cpp)
  set(imgui_backend_sources ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
                            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
  find_package(X11 REQUIRED)

  add_library(imgui STATIC ${imgui_sources} ${imgui_misc_sources}
                           ${imgui_backend_sources})
  target_link_libraries(imgui PUBLIC X11)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/backends
                                          ${imgui_SOURCE_DIR})

  # Override compiler flags for imgui target
  target_compile_options(
    imgui
    PRIVATE $<$<CONFIG:Debug>: -O2> # Force optimization level 2 even in Debug
            $<$<CONFIG:Debug>: -DNDEBUG> # Define NDEBUG in Debug
            $<$<NOT:$<CONFIG:Debug>>: -O2> # Use O2 for other build types
  )

endfunction()

installdeps()

find_package(glfw3 REQUIRED 1.3)
find_package(Vulkan REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(assimp REQUIRED)

find_package(Vulkan COMPONENTS glslc)
find_program(
  glslc_executable
  NAMES glslc
  HINTS Vulkan::glslc)
