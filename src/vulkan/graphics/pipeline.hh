#pragma once

#include <fwd.h>

class CommandPool;
class SurfaceManager;

class Pipeline {
public:
  Pipeline(SurfaceManager &surface_manager)
      : surface_manager(surface_manager) {};
  void createRenderPass();
  void createGraphicPipeline();
  void createFramebuffers();
  void cleanup();

private:
  SurfaceManager &surface_manager;

  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;
  VkPipeline graphicsPipeline;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  friend CommandPool;
};
