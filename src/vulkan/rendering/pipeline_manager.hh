#pragma once

#include "api/parameters.hh"
#include "fwd.h"
#include "vulkan/base_manager.hh"
#include <vulkan/vulkan_core.h>

class CommandPool;
class SurfaceManager;

class PipelineManager : public BaseManager {
public:
  PipelineManager(VulkanContext &context)
      : BaseManager(context), pipeline(context.pipeline) {}

  void createRenderPass();
  void createGraphicPipeline(Flim::Renderer &renderer);
  void cleanup();

private:
  Pipeline &pipeline;
};
