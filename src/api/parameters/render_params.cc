#include "render_params.hh"

namespace Flim {

bool RenderParams::usable() const {
  return !vertexShader.code.empty() && !fragmentShader.code.empty();
}

void RenderParams::invalidate() { version++; };

} // namespace Flim
