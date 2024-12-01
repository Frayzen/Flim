#include "pipeline_manager.hh"

#include "vulkan/context.hh"
#include "vulkan/rendering/shader_utils.hh"
#include <stdexcept>

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

void PipelineManager::createGraphicPipeline() {
  auto vertShaderCode = readFile("shaders/shader.vert.spv");
  auto fragShaderCode = readFile("shaders/shader.frag.spv");
  pipeline.vertShaderModule = createShaderModule(context, vertShaderCode);
  pipeline.fragShaderModule = createShaderModule(context, fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = pipeline.vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = pipeline.fragShaderModule;
  fragShaderStageInfo.pName = "main";

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

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
                                                          //
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
  rasterizer.depthClampEnable = VK_FALSE;

  // If set to VK_TRUE, then fragments that are beyond the near and far planes
  // are clamped to them as opposed to discarding them.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // If set to VK_TRUE, then geometry never passes through the rasterizer stage.
  // This basically disables any output to the framebuffer.
  rasterizer.rasterizerDiscardEnable = VK_FALSE;

  // determines how fragments are generated for geometry.
  //  VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
  //  VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
  //  VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

  // any line thicker than 1.0f requires you to enable the wideLines GPU feature
  rasterizer.lineWidth = 1.0f;

  // cullMode variable determines the type of face culling to use
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;            // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr,
                             &pipeline.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr; // Optional
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipeline.pipelineLayout;
  pipelineInfo.renderPass = pipeline.renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1;              // Optional
                                                    //
  if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &pipeline.graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(context.device, pipeline.fragShaderModule, nullptr);
  vkDestroyShaderModule(context.device, pipeline.vertShaderModule, nullptr);
}

void PipelineManager::createRenderPass() {
  // attachment refers to a memory resource (such as an image or buffer) used as
  // an input or output during rendering

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = context.swapChain.swapChainImageFormat;
  // we're not doing anything with multisampling yet, so we'll stick to 1 sample
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  // clear the framebuffer to black before drawing a new frame
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // we're interested in seeing the rendered triangle on the screen
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  // Our context won't do anything with the stencil buffer, so the results
  // of loading and storing are irrelevant
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // initialLayout specifies which layout the image will have before the render
  // pass begins
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // finalLayout specifies the layout to automatically transition to when the
  // render pass finishes
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  // attachment parameter specifies which attachment to reference by its index
  // in the attachment descriptions array.
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  // VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the
  // render pass depending on whether it is specified in srcSubpass or
  // dstSubpass
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  // index 0 refers to our subpass
  // dstSubpass must always be higher than srcSubpass (unless one of the
  // subpasses is VK_SUBPASS_EXTERNAL)
  dependency.dstSubpass = 0;
  // We need to wait for the swap chain to finish reading from the image before
  // we can access it
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  // The operations that should wait on this are in the color attachment stage
  // and involve the writing of the color attachment. These settings will
  // prevent the transition from hcontext until it's actually necessary (and
  // allowed): when we want to start writing colors to it.
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr,
                         &pipeline.renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void PipelineManager::createFramebuffers() {
  auto &swapChainImageViews = context.swapChain.swapChainImageViews;
  context.swapChain.swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {swapChainImageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = pipeline.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    // May differ from HEIGHT and WIDTH
    framebufferInfo.width = context.swapChain.swapChainExtent.width;
    framebufferInfo.height = context.swapChain.swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr,
                            &context.swapChain.swapChainFramebuffers[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void PipelineManager::cleanup() {
  vkDestroyPipeline(context.device, pipeline.graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(context.device, pipeline.pipelineLayout, nullptr);
  vkDestroyRenderPass(context.device, pipeline.renderPass, nullptr);
}
