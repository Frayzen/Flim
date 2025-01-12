#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace Flim {
class BaseParams;
class Mesh;
}; // namespace Flim

class DescriptorHolder {
public:
  DescriptorHolder(Flim::BaseParams *params, bool computeHolder)
      : params(params), descriptorPool(0), descriptorSetLayout(0),
        isComputeHolder(computeHolder) {};

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  void printBufferIds() const;

protected:
  void setupDescriptors();
  void cleanupDescriptors();
  Flim::BaseParams *params;

private:
  bool isComputeHolder; // if this is a computer class

  void createDescriptorSetLayout();
  void createDescriptorSets();
  void createDescriptorPool();

  int getDescriptorsSize() const;
};
