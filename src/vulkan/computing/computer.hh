#pragma once

#include "api/parameters/compute_params.hh"
#include "vulkan/rendering/renderer.hh"
class Computer {
public:
  Computer(Computer &) = delete;
  Computer() = delete;
  Computer(Flim::ComputeParams &params, Renderer &renderer)
      : params(params), renderer(renderer) {};

  Flim::ComputeParams &params;
  Renderer &renderer;
};
