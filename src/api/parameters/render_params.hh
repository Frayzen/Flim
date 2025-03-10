#pragma once

#include "api/shaders/shader.hh"
#include "api/tree/camera.hh"
#include "base_params.hh"
#include <string>

namespace Flim {

#define BINDING_DEFAULT_VIEWS_UNIFORM 0
#define BINDING_DEFAULT_MATERIALS_UNIFORM 1
#define BINDING_DEFAULT_VERTICES_ATTRIBUTES 0
#define BINDING_DEFAULT_INSTANCES_ATTRIBUTE 3

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

  static RenderParams DefaultParams(const Mesh &m, const Camera &cam);

private:
  RenderParams clone();
  RenderParams(const RenderParams &) = default;
  friend class ::Renderer;
};

} // namespace Flim
