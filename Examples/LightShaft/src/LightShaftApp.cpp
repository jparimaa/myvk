#include "LightShaftApp.h"
#include "LightShaftCommon.h"

#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Pipeline.h"
#include "fw/RenderPass.h"

#include <array>

LightShaftApp::~LightShaftApp()
{
    vkDestroyFence(m_logicalDevice, m_renderBufferFence, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_textureDescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool LightShaftApp::initialize()
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    m_objectRenderPass.initialize(&m_camera);
    m_lightShaftPrepass.initialize(m_objectRenderPass.getMatrixDescriptorSetLayout(), &m_lightTransformation);

    createRenderPass();
    CHECK(fw::API::initializeSwapChainWithDefaultFramebuffer(m_renderPass));
    createDescriptorSetLayouts();
    createPipeline();
    CHECK(m_sampler.create(VK_COMPARE_OP_NEVER));
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffer();
    createFence();
    CHECK(fw::API::initializeGUI(m_descriptorPool));

    m_cameraController.setCamera(&m_camera);
    glm::vec3 initPos(0.0f, 10.0f, 40.0f);
    m_cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    m_camera.setPosition(initPos);

    m_lightTransformation.setScale(3.0f);

    return true;
}

void LightShaftApp::update()
{
    m_cameraController.update();
    m_objectRenderPass.update();
    m_lightShaftPrepass.update(m_camera);

    glm::vec4 pos(m_lightTransformation.getPosition(), 1.0f);
    const glm::mat4& world = m_lightTransformation.getWorldMatrix();
    const glm::mat4& view = m_camera.getViewMatrix();
    const glm::mat4& proj = m_camera.getProjectionMatrix();

    pos = proj * view * world * pos;

    VkExtent2D size = fw::API::getSwapChainExtent();
    glm::vec2 ndc{pos.x / pos.w, pos.y / pos.w};
    glm::vec2 uv2{(0.5 * ndc.x) + (0.5), (0.5 * ndc.y) + (0.5)};

    m_shaderParameters.lightPosScreen = uv2;

    updateCommandBuffers();
}

void LightShaftApp::onGUI()
{
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

    glm::vec3 p = m_camera.getTransformation().getPosition();
    ImGui::Text("Camera position: %.1f %.1f %.1f", p.x, p.y, p.z);
    ImGui::Text("%.2f ms/frame (%.0f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::SliderInt("Samples", &m_shaderParameters.numSamples, 10, 200);
    ImGui::SliderFloat("Density", &m_shaderParameters.density, 0.0f, 1.0f);
    ImGui::SliderFloat("Weight", &m_shaderParameters.weight, 0.0f, 0.5f);
    ImGui::SliderFloat("Decay", &m_shaderParameters.decay, 0.9f, 1.0f);
    ImGui::SliderFloat("Exposure", &m_shaderParameters.exposure, 0.0f, 1.0f);

#ifndef WIN32
#pragma GCC diagnostic pop
#endif
}

void LightShaftApp::createRenderPass()
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription colorAttachment = fw::RenderPass::getColorAttachment();
    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = fw::ui32size(attachments);
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
}

void LightShaftApp::createDescriptorSetLayouts()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings(2);

    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr; // Optional

    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = fw::ui32size(bindings);
    textureLayoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &textureLayoutInfo, nullptr, &m_textureDescriptorSetLayout));
}

void LightShaftApp::createPipeline()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ShaderParameters);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts{m_textureDescriptorSetLayout};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "lightshaft.vert.spv", c_shaderFolder + "lightshaft.frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
        for (const auto& info : shaderStages)
        {
            vkDestroyShaderModule(m_logicalDevice, info.module, nullptr);
        }
    });

    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = fw::Pipeline::getInputAssemblyState();

    VkViewport viewport = fw::Pipeline::getViewport();
    VkRect2D scissor = fw::Pipeline::getScissorRect();
    VkPipelineViewportStateCreateInfo viewportState = fw::Pipeline::getViewportState(&viewport, &scissor);

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.front = depthStencilState.back;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

    VkPipelineRasterizationStateCreateInfo rasterizationState = fw::Pipeline::getRasterizationState();
    rasterizationState.cullMode = VK_CULL_MODE_NONE;

    VkPipelineMultisampleStateCreateInfo multisampleState = fw::Pipeline::getMultisampleState();
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(&colorBlendAttachmentState);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = fw::ui32size(shaderStages);
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));
}

void LightShaftApp::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 3;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void LightShaftApp::createDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_textureDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_textureDescriptorSet));

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    VkDescriptorImageInfo preLightShaftimageInfo{};
    preLightShaftimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    preLightShaftimageInfo.imageView = m_lightShaftPrepass.getOutputImageView();
    preLightShaftimageInfo.sampler = m_sampler.getSampler();

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_textureDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &preLightShaftimageInfo;

    VkDescriptorImageInfo objectPassImageInfo{};
    objectPassImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    objectPassImageInfo.imageView = m_objectRenderPass.getOutputImageView();
    objectPassImageInfo.sampler = m_sampler.getSampler();

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_textureDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &objectPassImageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void LightShaftApp::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = fw::API::getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &m_commandBuffer));
}

void LightShaftApp::createFence()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_renderBufferFence));
    fw::API::setRenderBufferFence(m_renderBufferFence);
}

void LightShaftApp::updateCommandBuffers()
{
    const std::vector<VkFramebuffer>& swapChainFramebuffers = fw::API::getSwapChainFramebuffers();
    uint32_t currentIndex = fw::API::getCurrentSwapChainImageIndex();
    VkFramebuffer framebuffer = swapChainFramebuffers[currentIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    vkWaitForFences(m_logicalDevice, 1, &m_renderBufferFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_logicalDevice, 1, &m_renderBufferFence);

    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

    m_objectRenderPass.writeRenderCommands(m_commandBuffer);
    m_lightShaftPrepass.writeRenderCommands(m_commandBuffer, m_objectRenderPass.getRenderObjects());

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.framebuffer = framebuffer;

    vkCmdPushConstants(m_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShaderParameters), &m_shaderParameters);

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_textureDescriptorSet, 0, nullptr);
    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_commandBuffer);

    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));

    fw::API::setNextCommandBuffer(m_commandBuffer);
}
