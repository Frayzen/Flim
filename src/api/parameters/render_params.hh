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

  AttributeDescriptor &setAttribute(int binding,
                                    AttributeRate rate = AttributeRate::VERTEX);
  // Validators
  bool usable() const override;
  void invalidate();

  RenderParams(const RenderParams &) = default;
private:
  RenderParams clone();
  friend class ::Renderer;
};

} // namespace Flim
