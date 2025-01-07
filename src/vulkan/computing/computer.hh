#pragma once

#include "api/parameters/compute_params.hh"
#include "vulkan/buffers/descriptor_holder.hh"

class Computer : DescriptorHolder {
public:
  Computer(Computer &) = delete;
  Computer() = delete;
  Computer(Flim::ComputeParams &params)
      : DescriptorHolder(params, true), params(params) {};

  void setup();
  void cleanup();

  Flim::ComputeParams &params;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

private:
  void createPipeline();
};
