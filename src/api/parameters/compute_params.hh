#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"

namespace Flim {
class ComputeParams : public BaseParams {
public:
  ComputeParams() : mainFunction("main") {};

  Shader shader;
  std::string mainFunction;

  void linkWriteableAttribute(int fromBinding, int toBinding);

  // Validators
  bool usable() const;
};

} // namespace Flim
