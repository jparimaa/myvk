#include "PostLightShaft.h"
#include "LightShaftCommon.h"

#include "fw/Model.h"
#include "fw/API.h"
#include "fw/Constants.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/RenderPass.h"
#include "fw/Macros.h"
#include "fw/Pipeline.h"

#include <array>

PostLightShaft::~PostLightShaft()
{
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_textureDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool PostLightShaft::initialize(VkImageView inputImageView)
{
    m_logicalDevice = fw::Context::getLogicalDevice();
    m_inputImageView = inputImageView;
    m_sampler.create(VK_COMPARE_OP_NEVER);
    createRenderPass();
    createDescriptorSetLayouts();
    createPipeline();
    createDescriptorPool();
    createDescriptorSet();
    return true;
}

void PostLightShaft::update(const fw::Camera& camera, const fw::Transformation& light)
{
    glm::vec4 pos(light.getPosition(), 1.0f);
    const glm::mat4& world = light.getWorldMatrix();
    const glm::mat4& view = camera.getViewMatrix();
    const glm::mat4& proj = camera.getProjectionMatrix();

    pos = proj * view * world * pos;

    VkExtent2D size = fw::API::getSwapChainExtent();
    glm::vec2 halfSize{size.width / 2.0f, size.height / 2.0f};
    glm::vec2 ndc{pos.x / pos.w, pos.y / pos.w};
    // Viewport transform
    glm::vec2 screen{(halfSize.x * ndc.x) + (halfSize.x), (halfSize.y * ndc.y) + (halfSize.y)};
    glm::vec2 uv{screen.x / size.width, screen.y / size.height};
    m_shaderParameters.lightPosScreen = uv;
}

void PostLightShaft::onGUI()
{
    ImGui::SliderInt("Samples", &m_shaderParameters.numSamples, 10, 200);
    ImGui::SliderFloat("Density", &m_shaderParameters.density, 0.0f, 1.0f);
    ImGui::SliderFloat("Weight", &m_shaderParameters.weight, 0.0f, 0.5f);
    ImGui::SliderFloat("Decay", &m_shaderParameters.decay, 0.9f, 1.0f);
    ImGui::SliderFloat("Exposure", &m_shaderParameters.exposure, 0.0f, 1.0f);
}

void PostLightShaft::writeRenderCommands(VkCommandBuffer cb, VkFramebuffer finalFramebuffer)
{
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
    renderPassInfo.framebuffer = finalFramebuffer;

    VkDeviceSize offsets[] = {0};

    vkCmdPushConstants(cb, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShaderParameters), &m_shaderParameters);

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_textureDescriptorSet, 0, nullptr);
    vkCmdDraw(cb, 3, 1, 0, 0);

    vkCmdEndRenderPass(cb);
}

void PostLightShaft::createRenderPass()
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

void PostLightShaft::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 0;
    textureLayoutBinding.descriptorCount = 1;
    textureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = 1;
    textureLayoutInfo.pBindings = &textureLayoutBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &textureLayoutInfo, nullptr, &m_textureDescriptorSetLayout));
}

void PostLightShaft::createPipeline()
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
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "post_lightshaft.vert.spv", c_shaderFolder + "post_lightshaft.frag.spv");

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

void PostLightShaft::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void PostLightShaft::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_textureDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_textureDescriptorSet));

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_inputImageView;
    imageInfo.sampler = m_sampler.getSampler();

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_textureDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}
