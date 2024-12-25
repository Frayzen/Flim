#pragma once

#include "api/parameters.hh"
#include "api/tree/instance_object.hh"
class DescriptorsManager {
public:
  DescriptorsManager() = default;
  void createDescriptorPool();
  void createDescriptorSetLayout();
  void createUniformBuffers();
  void createDescriptorSets();
  void updateUniformBuffer(const Flim::InstanceObject &object,
                           const Flim::CameraObject *cam);
  void updateDescriptors();
  Flim::DescriptorList* descriptors;
};
