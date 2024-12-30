#include "pipeline.hh"
#include "api/parameters.hh"
#include "vulkan/context.hh"
#include "api/render/mesh.hh"
#include "vulkan/rendering/renderer.hh"
#include <iostream>
#include <vulkan/vulkan_core.h>

void Pipeline::cleanup() {
  vkDestroyPipeline(context.device, pipeline, nullptr);
  vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);
}

inline std::vector<VkVertexInputBindingDescription> getBindingDescription() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Flim::Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  bindingDescriptions[1].binding = 1;
  bindingDescriptions[1].stride = sizeof(Matrix4f);
  bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

  return bindingDescriptions;
}

inline std::vector<VkVertexInputAttributeDescription>
getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3 +
                                                                       4 * 1);

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Flim::Vertex, pos);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Flim::Vertex, normal);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(Flim::Vertex, uv);

  for (int i = 0; i < 4; i++) { // attribute for a Matrix4f
    attributeDescriptions[3 + i].binding = 1;
    attributeDescriptions[3 + i].location = 3 + i;
    attributeDescriptions[3 + i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3 + i].offset = sizeof(Vector4f) * i;
  }

  return attributeDescriptions;
}

/*
Shader stages: the shader modules that define the functionality of the
programmable stages of the graphics pipeline

Fixed-function state: all of the structures that define the fixed-function
stages of the pipeline, like input assembly, rasterizer, viewport and color
blending

Pipeline layout: the uniform and push values referenced by the shader that can
be updated at draw time

Render pass: the attachments referenced by the pipeline stages and their usage
*/

static VkShaderModule createShaderModule(VulkanContext &context,
                                         const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(context.device, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
  return shaderModule;
}

void Pipeline::create() {
  Flim::RenderParams &params = renderer.params;
  vertShaderModule = createShaderModule(context, params.vertexShader.code);
  fragShaderModule = createShaderModule(context, params.fragmentShader.code);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = params.vertexShader.entry.data();

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = params.fragmentShader.entry.data();

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};

  // Enable dynamic state to allow runtime change of size of the viewport, line
  // width, blend constants...
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // Types:
  //    VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
  //    VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without
  //    reuse VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is
  //    used as start vertex for the next line
  //    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices
  //    without reuse VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third
  //    vertex of every triangle are used as first two vertices of the next
  //    triangle
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  // If set to VK_TRUE, then fragments that are beyond the near and far planes
  // are clamped to them as opposed to discarding them.
  rasterizer.depthClampEnable = VK_FALSE;
  // If set to VK_TRUE, then geometry never passes through the rasterizer stage.
  // This basically disables any output to the framebuffer.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;

  // determines how fragments are generated for geometry.
  //  VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
  //  VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
  //  VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
  /* rasterizer.polygonMode = VK_POLYGON_MODE_FILL; */
  rasterizer.polygonMode = Flim::renderModeToPolygonMode(params.mode);
  /* rasterizer.polygonMode = VK_POLYGON_MODE_POINT; */

  // any line thicker than 1.0f requires you to enable the wideLines GPU feature
  rasterizer.lineWidth = 3.0f;

  // cullMode variable determines the type of face culling to use
  rasterizer.cullMode = renderer.params.useBackfaceCulling
                            ? VK_CULL_MODE_FRONT_BIT
                            : VK_CULL_MODE_NONE;
  // frontFace variable specifies the vertex order for faces to be considered
  // front-facing
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f;          // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

  // MULTISAMPLING (to smoothen triangles)
  // Disable multisampling for now
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;          // Optional
  multisampling.pSampleMask = nullptr;            // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE;      // Optional

  // COLOR BLENDING: mixing already existing color with the new one (usefull for
  // stained glasses for example) Disabled for now
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &renderer.descriptorSetLayout;

  if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  auto bindingDescriptions = getBindingDescription();
  auto attributeDescriptions = getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;  // should we discard invalid ones
  depthStencil.depthWriteEnable = VK_TRUE; // should we overwrite valid ones
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  // Enabled if depth is relevant only in a certain range
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.maxDepthBounds = 1.0f;
  // For stencil test
  depthStencil.stencilTestEnable = VK_FALSE;

  VkPipelineRenderingCreateInfoKHR pipelinInfoKHR{};
  pipelinInfoKHR.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
  pipelinInfoKHR.colorAttachmentCount = 1;
  pipelinInfoKHR.pColorAttachmentFormats =
      &context.swapChain.swapChainImageFormat;
  pipelinInfoKHR.depthAttachmentFormat = context.depthImage.format;

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = nullptr;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineIndex = -1;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pNext = &pipelinInfoKHR;

  if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }
  std::cout << "Graphic pipeline (re)created" << std::endl;
  vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
  vkDestroyShaderModule(context.device, vertShaderModule, nullptr);
}
