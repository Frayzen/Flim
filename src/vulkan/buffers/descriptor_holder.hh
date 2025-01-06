#pragma once

#include "fwd.hh"
#include <map>
#include <vector>

namespace Flim {
class BaseParams;
class Mesh;
}; // namespace Flim

class DescriptorHolder {
public:
  DescriptorHolder(Flim::Mesh &mesh, Flim::BaseParams &params,
                   bool computeHolder)
      : mesh(mesh), params(params), descriptorPool(0), descriptorSetLayout(0),
        isComputeHolder(computeHolder) {};

  std::map<int, std::vector<Buffer>> uniforms;
  std::map<int, std::vector<void *>> mappedUniforms;

  std::map<int, std::vector<Buffer>> attributes;
  std::map<int, std::vector<void *>> mappedAttributes;

  Flim::Mesh &mesh;

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
