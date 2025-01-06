#include "shader.hh"
#include "vulkan/context.hh"
#include <fstream>

namespace Flim {

static std::vector<char> readFile(const std::string &path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open " + path);
  }
  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

Shader::Shader(std::string path, std::string entry) : entry(entry) {
  code = readFile(path);
}

/*
Shader stages: the shader modules that define the functionality of the
programmable stages of the graphics pipeline

Fixed-function state: all of the structures that define the fixed-function
stages of the pipeline, like input assembly, rasterizer, viewport and color
blending

Pipeline layout: the uniform and push values referenced by the shader that can
be updated at draw time

Render pass: the attachments referenced by the pipeline stages and their usage
*/

VkShaderModule Shader::createShaderModule() {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(context.device, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
  return shaderModule;
}

} // namespace Flim
