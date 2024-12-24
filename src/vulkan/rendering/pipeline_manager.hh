#pragma once

#include <fwd.hh>
#include "vulkan/context.hh"

#include <vulkan/vulkan_core.h>

class CommandPool;
class SurfaceManager;

class PipelineManager {
public:
  PipelineManager() : pipeline(context.pipeline) {};

  void createGraphicPipeline();
  void cleanup();

private:
  Pipeline &pipeline;
};
