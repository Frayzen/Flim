#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"

namespace Flim {
class ComputeParams : public BaseParams {
public:
  ComputeParams() : mainFunction("main") {};

  Shader shader;
  std::string mainFunction;

  ComputeParams clone();

  void linkWriteableAttribute(int fromBinding, int toBinding);

  // Validators
  bool usable() const;
};

} // namespace Flim
