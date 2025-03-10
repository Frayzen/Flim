#include "render_params.hh"
#include "api/tree/camera.hh"
#include "utils/checks.hh"
#include "vulkan/buffers/params_utils.hh"

namespace Flim {

bool RenderParams::usable() const {
  return !vertexShader.code.empty() && !fragmentShader.code.empty();
}

void RenderParams::invalidate() { version++; };

AttributeDescriptor &RenderParams::setAttribute(int binding,
                                                AttributeRate rate) {
  CHECK(!attributes.contains(binding),
        "Cannot 'set' an existing attribute, please use update instead");
  std::shared_ptr<AttributeDescriptor> ptr =
      std::make_shared<AttributeDescriptor>(binding, rate);
  attributes[binding] = ptr;
  return *ptr;
}

RenderParams RenderParams::clone() {
  RenderParams cloned(*this);
  for (auto attr : attributes) {
    cloned.attributes[attr.first]->bufferId = attr.second->bufferId;
  }
  for (auto attr : uniforms) {
    cloned.uniforms[attr.first]->bufferId = attr.second->bufferId;
  }
  return cloned;
}

RenderParams RenderParams::DefaultParams(const Mesh& m, const Camera& cam) {
  RenderParams params("Default");

  // Uniforms
  ParamsUtils::createViewMatrixUniform(params, BINDING_DEFAULT_VIEWS_UNIFORM, m, cam);
  ParamsUtils::createMaterialUniform(params, BINDING_DEFAULT_MATERIALS_UNIFORM, m);

  // Attributes
  ParamsUtils::createVerticesAttribute(params, BINDING_DEFAULT_VERTICES_ATTRIBUTES);
  ParamsUtils::createInstanceMatrixAttribute(params, BINDING_DEFAULT_INSTANCES_ATTRIBUTE);

  params.vertexShader = Shader("shaders/default.vert.spv");
  params.fragmentShader = Shader("shaders/default.frag.spv");
  return params;
}

} // namespace Flim
