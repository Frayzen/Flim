find_package(Vulkan COMPONENTS glslc)
find_program(
  glslc_executable
  NAMES glslc
  HINTS Vulkan::glslc)

function(add_shaders TARGET_NAME SHADER_SOURCE)
  # Set input and output paths
  set(INPUT_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE}")
  set(OUTPUT_SPV "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_SOURCE}.spv")
  
  add_custom_command(
    OUTPUT ${OUTPUT_SPV}
    COMMAND ${glslc_executable} ${INPUT_SHADER} -o ${OUTPUT_SPV}
    DEPENDS ${INPUT_SHADER}
    COMMENT "Compiling Shader [${SHADER_SOURCE}]"
    VERBATIM
  )
  
  add_custom_target(
    ${TARGET_NAME} ALL
    DEPENDS ${OUTPUT_SPV}
  )
endfunction()

add_shaders("VertexShader" default.vert)
add_shaders("FragmentShader" default.frag)
add_shaders("ComputeShader" default.comp)
