#include "computer.hh"
#include "consts.hh"
#include "vulkan/context.hh"
#include <iostream>
#include <vulkan/vulkan_core.h>

void Computer::createPipeline() {
  auto shaderModule = params.shader.createShaderModule();

  VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
  computeShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = shaderModule;
  computeShaderStageInfo.pName = params.mainFunction.c_str();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline layout!");
  }

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo,
                               nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline!");
  }
  vkDestroyShaderModule(context.device, shaderModule, nullptr);
}

void Computer::setup() {
  setupDescriptors();
  createPipeline();
}

void Computer::cleanup() {
  vkDestroyPipeline(context.device, pipeline, nullptr);
  vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);
  cleanupDescriptors();
}
