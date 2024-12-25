#pragma once

#include "api/parameters.hh"
#include "api/tree/instance_object.hh"
class DescriptorsManager {
public:
  DescriptorsManager() = default;
  void createDescriptorSetLayout();
  void setupUniforms();
  void cleanup();
  void createDescriptorSets();
  void createDescriptorPool();
  void updateUniforms(const Flim::InstanceObject &obj,
                      const Flim::CameraObject &cam);
  Flim::DescriptorList *descriptors;
};
