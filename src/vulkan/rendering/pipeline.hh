#pragma once

#include <vector>
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
  std::vector<VkVertexInputBindingDescription> getBindingDescription();
  std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  friend Renderer;
};
