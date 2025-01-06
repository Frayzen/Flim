#include "computer.hh"

void Computer::createPipeline()
{
  VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
  computeShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = params.shader.createShaderModule();
  computeShaderStageInfo.pName = params.mainFunction.c_str();

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo,
                               nullptr, &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline!");
  }
}

void Computer::setup() {
  setupDescriptors();
  createPipeline();
}

void Computer::cleanup()
{
  cleanupDescriptors();
}
