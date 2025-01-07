#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace Flim {
class BaseParams;
class Mesh;
}; // namespace Flim

class DescriptorHolder {
public:
  DescriptorHolder(Flim::BaseParams &params, bool computeHolder)
      : params(params), descriptorPool(0), descriptorSetLayout(0),
        isComputeHolder(computeHolder) {};

  std::vector<VkDescriptorSet> descriptorSets;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;

protected:
  void setupDescriptors();
  void cleanupDescriptors();

private:
  bool isComputeHolder; // if this is a computer class
  Flim::BaseParams &params;

  void createDescriptorSetLayout();
  void createDescriptorSets();
  void createDescriptorPool();
  friend class CommandPoolManager;
};
