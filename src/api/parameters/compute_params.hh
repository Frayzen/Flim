#pragma once

#include "api/shaders/shader.hh"
#include "base_params.hh"

class Computer;

namespace Flim {
class ComputeParams : public BaseParams {
public:
  ComputeParams() : BaseParams(), mainFunction("main") {};

  Shader shader;
  std::string mainFunction;

  void linkWriteableAttribute(int fromBinding, int toBinding);

  // Validators
  bool usable() const;

  ComputeParams(const ComputeParams &from)
      : BaseParams(from), shader(from.shader), mainFunction(from.mainFunction) {
    for (auto &attr : from.attributes) {
      attributes[attr.first] =
          attr.second->clone(true); // we keep the value here
    }
  }
private:
  ComputeParams clone();
  friend class ::Computer;
};

} // namespace Flim
