#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"

namespace Flim {
class ComputeParams : public BaseParams {
public:
  ComputeParams() = default;

  Shader shader;

  void linkWriteableAttribute(int fromBinding, int toBinding);

  // Validators
  bool usable() const;
};

} // namespace Flim
