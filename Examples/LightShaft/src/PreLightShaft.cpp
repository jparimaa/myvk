#include "PreLightShaft.h"

#include "fw/API.h"
#include "fw/Constants.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/RenderPass.h"
#include "fw/Macros.h"
#include "fw/Pipeline.h"

#include <array>

namespace
{
const VkDeviceSize c_colorBufferSize = sizeof(float) * 4;
} // unnamed

PreLightShaft::~PreLightShaft()
{
    vkDestroyImageView(m_logicalDevice, m_imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);
    vkDestroyFramebuffer(m_logicalDevice, m_framebuffer, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_matrixDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_colorDescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool PreLightShaft::init(uint32_t width, uint32_t height)
{
    m_logicalDevice = fw::Context::getLogicalDevice();
    m_width = width;
    m_height = height;
    createRenderPass();
    createFramebuffer();
    createDescriptorSetLayouts();
    createPipeline();
    createDescriptorPool();
    createDescriptorSets();
    return true;
}

void PreLightShaft::createRenderPass()
{
    VkAttachmentDescription colorAttachment = fw::RenderPass::getColorAttachment();
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

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

void PreLightShaft::createFramebuffer()
{
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    CHECK(m_image.create(m_width, m_height, c_format, 0, usage, 1));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = c_format;
    viewInfo.flags = 0;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.image = m_image.getHandle();

    VK_CHECK(vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, &m_imageView));

    VkFormat depthFormat = fw::Constants::depthFormat;
    VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CHECK(m_depthImage.create(m_width, m_height, depthFormat, 0, depthImageUsage, 1));
    CHECK(m_depthImage.createView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depthImageView));
    CHECK(m_depthImage.transitLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

    std::array<VkImageView, 2> attachments = {m_imageView, m_depthImageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = fw::ui32size(attachments);
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_width;
    framebufferInfo.height = m_height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffer));

    VkCommandBuffer commandBuffer = fw::Command::beginSingleTimeCommands();

    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.image = m_image.getHandle();
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    fw::Command::endSingleTimeCommands(commandBuffer);
}

void PreLightShaft::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding matrixLayoutBinding{};
    matrixLayoutBinding.binding = 0;
    matrixLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matrixLayoutBinding.descriptorCount = 1;
    matrixLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    matrixLayoutBinding.pImmutableSamplers = nullptr; // Optional

    std::array<VkDescriptorSetLayoutBinding, 1> matrixBindings = {matrixLayoutBinding};
    VkDescriptorSetLayoutCreateInfo matrixLayoutInfo{};
    matrixLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    matrixLayoutInfo.bindingCount = fw::ui32size(matrixBindings);
    matrixLayoutInfo.pBindings = matrixBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &matrixLayoutInfo, nullptr, &m_matrixDescriptorSetLayout));

    VkDescriptorSetLayoutBinding colorLayoutBinding{};
    colorLayoutBinding.binding = 0;
    colorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    colorLayoutBinding.descriptorCount = 1;
    colorLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    colorLayoutBinding.pImmutableSamplers = nullptr; // Optional

    std::array<VkDescriptorSetLayoutBinding, 1> colorBindings = {colorLayoutBinding};
    VkDescriptorSetLayoutCreateInfo colorLayoutInfo{};
    colorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    colorLayoutInfo.bindingCount = fw::ui32size(colorBindings);
    colorLayoutInfo.pBindings = colorBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &colorLayoutInfo, nullptr, &m_colorDescriptorSetLayout));
}

void PreLightShaft::createPipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "pre_lightshaft.vert.spv", c_shaderFolder + "pre_lightshaft.frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
        for (const auto& info : shaderStages)
        {
            vkDestroyShaderModule(m_logicalDevice, info.module, nullptr);
        }
    });

    VkVertexInputBindingDescription vertexDescription = fw::Pipeline::getVertexDescription();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = fw::Pipeline::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputState = fw::Pipeline::getVertexInputState(&vertexDescription, &attributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = fw::Pipeline::getInputAssemblyState();

    VkViewport viewport = fw::Pipeline::getViewport();
    VkRect2D scissor = fw::Pipeline::getScissorRect();
    VkPipelineViewportStateCreateInfo viewportState = fw::Pipeline::getViewportState(&viewport, &scissor);

    VkPipelineRasterizationStateCreateInfo rasterizationState = fw::Pipeline::getRasterizationState();
    VkPipelineMultisampleStateCreateInfo multisampleState = fw::Pipeline::getMultisampleState();
    VkPipelineDepthStencilStateCreateInfo depthStencilState = fw::Pipeline::getDepthStencilState();
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(&colorBlendAttachmentState);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts{m_matrixDescriptorSetLayout, m_colorDescriptorSetLayout};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

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

void PreLightShaft::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void PreLightShaft::createDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_colorDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_colorDescriptorSet));

    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CHECK(m_uniformBuffer.create(c_colorBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_uniformBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = c_colorBufferSize;

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_colorDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void PreLightShaft::writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.2f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();

    VkDeviceSize offsets[] = {0};

    renderPassInfo.framebuffer = m_framebuffer;

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    float color[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    m_uniformBuffer.setData(c_colorBufferSize, color);

    for (const RenderObject& ro : renderObjects)
    {
        VkBuffer vb = ro.vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
        vkCmdBindIndexBuffer(cb, ro.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        std::vector<VkDescriptorSet> sets{ro.matrixDescriptorSet, m_colorDescriptorSet};
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, fw::ui32size(sets), sets.data(), 0, nullptr);
        vkCmdDrawIndexed(cb, ro.numIndices, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(cb);
}
