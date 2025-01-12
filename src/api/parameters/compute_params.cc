#include "compute_params.hh"

namespace Flim {

ComputeParams ComputeParams::clone() {
  ComputeParams other(*this);
  for (auto &attr : attributes) {
    attributes[attr.first] =
        attr.second->clone(); // we update the id of the provided one
  }
  return other;
}
bool ComputeParams::usable() const { return !shader.code.empty(); }

}; // namespace Flim
