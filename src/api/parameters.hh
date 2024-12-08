#pragma once

#include "api/shaders/shader.hh"
namespace Flim {

struct Renderer {
  Shader vertexShader;
  Shader fragmentShader;
};

struct FlimParameters {
};

} // namespace Flim
