#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace Flim {
class BaseParams;
class Mesh;
}; // namespace Flim

// The holder of the descriptors of a Renderable object (uniform and attribute)
// The layout is handled in params. This takes the BaseParams which generates
// the descriptorss for the attributes and uniform and outputs a DescriptorPool
class DescriptorHolder {
public:
  DescriptorHolder(Flim::BaseParams &params, bool computeHolder)
      : params(params), descriptorPool(0), descriptorSetLayout(0),
        isComputeHolder(computeHolder) {};
  ~DescriptorHolder();

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  void printBufferIds() const;

protected:
  void setupDescriptors();
  Flim::BaseParams &params;

private:
  bool isComputeHolder; // if this is a computer class

  void createDescriptorSetLayout();
  void createDescriptorSets();
  void createDescriptorPool();

  int getDescriptorsSize() const;
};
