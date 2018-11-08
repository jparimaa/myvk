#include "LightShaftPrepass.h"

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

namespace
{
const size_t c_pushConstantsSize = sizeof(float) * 4;
} // namespace

LightShaftPrepass::~LightShaftPrepass()
{
    vkDestroyImageView(m_logicalDevice, m_imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);
    vkDestroyFramebuffer(m_logicalDevice, m_framebuffer, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool LightShaftPrepass::initialize(VkDescriptorSetLayout matrixDescriptorSetLayout, const fw::Transformation* light)
{
    m_sphereTransformation = light;
    m_matrixDescriptorSetLayout = matrixDescriptorSetLayout;
    m_logicalDevice = fw::Context::getLogicalDevice();

    createRenderPass();
    createFramebuffer();
    createPipeline();
    createDescriptorPool();
    createDescriptorSet();
    createRenderObject();

    return true;
}

void LightShaftPrepass::update(const fw::Camera& camera)
{
    m_ubo.world = m_sphereTransformation->getWorldMatrix();
    m_ubo.view = camera.getViewMatrix();
    m_ubo.proj = camera.getProjectionMatrix();
    m_sphereMatrixBuffer.setData(sizeof(m_ubo), &m_ubo);
}

void LightShaftPrepass::writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.framebuffer = m_framebuffer;

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    vkCmdPushConstants(cb, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, c_pushConstantsSize, &color);

    VkDeviceSize offsets[] = {0};

    // Sphere
    VkBuffer vb = m_sphere.vertexBuffer.getBuffer();
    vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
    vkCmdBindIndexBuffer(cb, m_sphere.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    std::vector<VkDescriptorSet> sets{m_sphere.matrixDescriptorSet};
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, fw::ui32size(sets), sets.data(), 0, nullptr);
    vkCmdDrawIndexed(cb, m_sphere.numIndices, 1, 0, 0, 0);

    // Objects
    color = {0.0, 0.0f, 0.0f, 0.0f};
    vkCmdPushConstants(cb, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, c_pushConstantsSize, &color);

    for (const RenderObject& ro : renderObjects)
    {
        VkBuffer vb = ro.vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
        vkCmdBindIndexBuffer(cb, ro.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        std::vector<VkDescriptorSet> sets{ro.matrixDescriptorSet};
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, fw::ui32size(sets), sets.data(), 0, nullptr);
        vkCmdDrawIndexed(cb, ro.numIndices, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(cb);
}

VkImageView LightShaftPrepass::getOutputImageView() const
{
    return m_imageView;
}

void LightShaftPrepass::createRenderPass()
{
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
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

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

void LightShaftPrepass::createFramebuffer()
{
    VkExtent2D extent = fw::API::getSwapChainExtent();
    uint32_t width = extent.width;
    uint32_t height = extent.height;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CHECK(m_image.create(width, height, c_format, 0, usage, 1));

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
    CHECK(m_depthImage.create(width, height, depthFormat, 0, depthImageUsage, 1));
    CHECK(m_depthImage.createView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depthImageView));
    CHECK(m_depthImage.transitLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

    std::array<VkImageView, 2> attachments = {m_imageView, m_depthImageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = fw::ui32size(attachments);
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
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

void LightShaftPrepass::createPipeline()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = c_pushConstantsSize;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts{m_matrixDescriptorSetLayout};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

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

void LightShaftPrepass::createDescriptorPool()
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

void LightShaftPrepass::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_matrixDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_sphere.matrixDescriptorSet));

    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CHECK(m_sphereMatrixBuffer.create(c_transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_sphereMatrixBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = c_transformMatricesSize;

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_sphere.matrixDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void LightShaftPrepass::createRenderObject()
{
    fw::Model model;
    CHECK(model.loadModel(c_assetsFolder + "lowpoly_sphere.obj"));

    fw::Model::Meshes meshes = model.getMeshes();
    uint32_t numMeshes = fw::ui32size(meshes);

    bool success = true;
    const fw::Mesh& mesh = meshes[0];

    success = success
        && m_sphere.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        && m_sphere.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_sphere.numIndices = fw::ui32size(mesh.indices);

    CHECK(success);
}
