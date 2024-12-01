#pragma once

#include "fwd.h"
#include "vulkan/base_manager.hh"

class CommandPool;
class SurfaceManager;

class PipelineManager : public BaseManager {
public:
  PipelineManager(VulkanContext &context)
      : BaseManager(context), pipeline(context.pipeline) {}

  void createRenderPass();
  void createGraphicPipeline();
  void cleanFramebuffers();
  void cleanup();

private:
  Pipeline &pipeline;
};
