find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

find_package(Vulkan COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
