#include "render_params.hh"

namespace Flim {

RenderParams RenderParams::clone(Mesh &m) {
  RenderParams rp(*this);
  rp.mesh = &m;
  for (auto &attr : rp.attributes) {
    rp.attributes[attr.first] = attr.second->clone();
    rp.attributes[attr.first]->mesh = &m;
  }
  return rp;
}

bool RenderParams::usable() const {
  return !vertexShader.code.empty() && !fragmentShader.code.empty();
}

void RenderParams::invalidate() { version++; };

AttributeDescriptor &RenderParams::setAttribute(int binding,
                                                AttributeRate rate) {
  std::shared_ptr<AttributeDescriptor> ptr =
      std::make_shared<AttributeDescriptor>(*mesh, binding, rate);
  attributes[binding] = ptr;
  return *ptr;
}

} // namespace Flim
