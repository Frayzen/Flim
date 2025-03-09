#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Flim {

class Shader {
public:
  Shader() = default;
  Shader(std::string path, std::string entry = "main");
  std::string entry;
  std::vector<char> code;

  VkShaderModule createShaderModule();
};

}; // namespace Flim
