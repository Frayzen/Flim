#pragma once

#include <fwd.h>

class Pipeline {
public:
  void createRenderPass();
  void createGraphicPipeline();
  void cleanup();

private:
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;
};
