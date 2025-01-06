#include "compute_params.hh"

namespace Flim {

bool ComputeParams::usable() const { return !shader.code.empty(); }

}; // namespace Flim
