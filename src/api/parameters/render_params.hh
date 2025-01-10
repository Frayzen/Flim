#pragma once

#include "api/render/mesh.hh"
#include "api/shaders/shader.hh"
#include "base_params.hh"

namespace Flim {

class RenderParams : public BaseParams {
public:
  RenderParams(Mesh &m) : mesh(&m) {};

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
  RenderParams clone(Mesh &m);

private:
  Mesh *mesh;
  RenderParams(const RenderParams &) = default;
};

} // namespace Flim
