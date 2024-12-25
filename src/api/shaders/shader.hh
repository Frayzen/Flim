#pragma once

#include <string>
#include <vector>

namespace Flim {

class Shader {
public:
  Shader() = default;
  Shader(std::string path, std::string entry = "main");
  std::string entry;
  std::vector<char> code;
};

}; // namespace Flim
