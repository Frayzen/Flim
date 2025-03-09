#include "compute_params.hh"

namespace Flim {

bool ComputeParams::usable() const { return !shader.code.empty(); }

ComputeParams ComputeParams::clone() {
  ComputeParams cloned(*this);
  for (auto attr : attributes)
  {
    cloned.attributes[attr.first]->bufferId = attr.second->bufferId;
  }
  for (auto attr : uniforms)
  {
    cloned.uniforms[attr.first]->bufferId = attr.second->bufferId;
  }
  return cloned;
}

}; // namespace Flim
