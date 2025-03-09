#pragma once
#include "api/parameters/render_params.hh"
#include "api/tree/camera.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include "vulkan/context.hh"
namespace Flim {

class ParamsUtils {

public:
  static AttributeDescriptor &
  createInstanceMatrixAttribute(RenderParams &params, int binding);

  static AttributeDescriptor &createVerticesAttribute(RenderParams &params,
                                                      int binding,
                                                      bool usesPos = true,
                                                      bool usesNormal = true,
                                                      bool usesUv = true);

  static UniformDescriptor &createViewMatrixUniform(RenderParams &params,
                                                    int binding, const Mesh &m,
                                                    const Camera &cam);

  static UniformDescriptor &createMaterialUniform(RenderParams &params,
                                                  int binding, const Mesh &m);
};
} // namespace Flim
