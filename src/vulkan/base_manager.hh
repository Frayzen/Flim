#pragma once

#include "fwd.h"
#include "vulkan/context.hh"

class BaseManager {
public:
  BaseManager(VulkanContext &context) : context(context) {};

protected:
  VulkanContext &context;
};
