#include "fvm_stack.hh"
#include "ast/fvm_ast.hh"
#include <iostream>

void FVMStack::setup() {
  size_t maxMatrix = 0;
  size_t maxSizeStack = 0;
  size_t curSizeStack = 0;

  std::vector<size_t> stack(depth);
  size_t stack_size = 0;

  for (auto node : rpn) {
    size_t node_size = node.height * node.width;
    maxMatrix = std::max(node_size, maxMatrix);

    if (node.left != NO_CHILD) {
      if (stack_size == 0) {
        // Handle error - stack underflow
        std::cerr << "Stack underflow!" << std::endl;
        return;
      }
      stack_size--;
      curSizeStack -= stack[stack_size];
    }

    if (node.right != NO_CHILD) {
      if (stack_size == 0) {
        std::cerr << "Stack underflow!" << std::endl;
        return;
      }
      stack_size--;
      curSizeStack -= stack[stack_size];
    }

    curSizeStack += node_size;
    maxSizeStack = std::max(maxSizeStack, curSizeStack);

    if (stack_size >= depth) {
      // Handle stack overflow
      std::cerr << "Stack overflow!" << std::endl;
      return;
    }
    stack[stack_size++] = node_size;
  }

  std::cout << "MAX MATRIX " << maxMatrix << std::endl;
  std::cout << "STACK SIZE " << maxSizeStack << std::endl;
  std::cout << stack_size << std::endl;
  assert(stack_size == 1);
}
