#pragma once

#include "api/parameters/compute_params.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include <Eigen/src/Core/Matrix.h>

class Computer : public DescriptorHolder {
public:
  Computer(Computer &) = delete;
  Computer() = delete;
  Computer(Vector3i dispatchAmount, Flim::ComputeParams &params)
      : DescriptorHolder(params, true), dispatchAmount(dispatchAmount),
        params(params) {};

  void setup();
  void update();
  void cleanup();

  Vector3i dispatchAmount;

  Flim::ComputeParams &params;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

private:
  void createPipeline();
};
