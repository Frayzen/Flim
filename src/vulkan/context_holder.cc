#include "vulkan/context_holder.hh"
#include <vulkan/vulkan_core.h>

static VulkanContext context = {};

VulkanContext &getContext() { return context; }
