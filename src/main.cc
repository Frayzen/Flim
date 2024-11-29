#include "app.hh"
#include "fwd.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

int main() {
  VulkanApplication app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
