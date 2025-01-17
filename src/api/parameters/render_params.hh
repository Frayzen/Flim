#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"
#include <string>

namespace Flim {

class RenderParams : public BaseParams {
public:
  RenderParams(std::string name) : BaseParams(name) {};
  RenderParams(std::string name, const RenderParams &from)
      : RenderParams(from) {
    name = name;
  };

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

private:
  RenderParams clone();
  RenderParams(const RenderParams &) = default;
  friend class ::Renderer;
};

} // namespace Flim
