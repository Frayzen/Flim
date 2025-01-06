#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"

namespace Flim {
class RenderParams : public BaseParams {
public:
  RenderParams() = default;

  Shader vertexShader;
  Shader fragmentShader;

  bool useBackfaceCulling = true;
  int version = 0;
  RenderMode mode = RenderMode::RENDERER_MODE_TRIS;

  // Validators
  bool usable() const override;
  void invalidate();
};

} // namespace Flim
