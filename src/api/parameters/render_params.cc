#include "render_params.hh"

namespace Flim {

RenderParams RenderParams::clone() {
  RenderParams other(*this);
  for (auto &attr : other.attributes) {
    attributes[attr.first] =
        attr.second->clone(); // we update the id of the provided one
  }
  return other;
}

bool RenderParams::usable() const {
  return !vertexShader.code.empty() && !fragmentShader.code.empty();
}

void RenderParams::invalidate() { version++; };

AttributeDescriptor &RenderParams::setAttribute(int binding,
                                                AttributeRate rate) {
  std::shared_ptr<AttributeDescriptor> ptr =
      std::make_shared<AttributeDescriptor>(binding, rate);
  attributes[binding] = ptr;
  return *ptr;
}

} // namespace Flim
