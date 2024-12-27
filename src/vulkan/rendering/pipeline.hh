#pragma once

#include <vulkan/vulkan_core.h>

class Renderer;
class Pipeline {
public:
  void cleanup();
  void create();

  Renderer &renderer;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;

private:
  Pipeline(Renderer &renderer) : renderer(renderer) {};
  friend Renderer;
};
