#include "shader.hh"
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

} // namespace Flim
