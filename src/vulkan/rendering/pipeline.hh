#pragma once

#include <vulkan/vulkan_core.h>

class Renderer;
class Pipeline {
public:
  void create();

  Renderer &renderer;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
  Pipeline(Renderer &renderer) : renderer(renderer) {};
  ~Pipeline();
};
