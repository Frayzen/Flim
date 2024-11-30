find_package(Vulkan COMPONENTS glslc)
find_program(
  glslc_executable
  NAMES glslc
  HINTS Vulkan::glslc)

function(add_shaders TARGET_NAME SHADER_SOURCE)
  set(SHADER_COMMAND)
  list(APPEND SHADER_COMMAND "glslc")
  list(APPEND SHADER_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE}")
  list(APPEND SHADER_COMMAND "-o")
  list(APPEND SHADER_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_SOURCE}.spv")

  add_custom_target(
    ${TARGET_NAME} ALL
    ${SHADER_COMMAND}
    COMMENT "Compiling Shader [${SHADER_SOURCE}]"
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE})
endfunction()

add_shaders("VertexShader" shader.vert)
add_shaders("FragmentShader" shader.frag)
