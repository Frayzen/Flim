#pragma once
#include "ast/fvm_ast.hh"
class FVMStack {

public:
  FVMStack(const std::vector<FVMAst::Node> &rpn, size_t depth)
      : rpn(rpn), depth(depth) {
    setup();
  }

private:
  void setup();

  const std::vector<FVMAst::Node> rpn;
  size_t depth;
};
