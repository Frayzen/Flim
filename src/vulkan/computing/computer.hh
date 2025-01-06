#pragma once

#include "api/parameters/compute_params.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/rendering/renderer.hh"
class Computer : DescriptorHolder {
public:
  Computer(Computer &) = delete;
  Computer() = delete;
  Computer(Flim::ComputeParams &params, Renderer &renderer)
      : DescriptorHolder(renderer.mesh, params, true), params(params),
        renderer(renderer) {};

  void setup();
  void cleanup();

  Flim::ComputeParams &params;
  Renderer &renderer;

  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

private:
  void createPipeline();
};
