#include "GBufferPass.h"
#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Mesh.h"
#include "fw/Model.h"
#include "fw/Pipeline.h"
#include "fw/RenderPass.h"

#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace
{
const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;
const VkFormat c_format = VK_FORMAT_R8G8B8A8_UNORM;
}

GBufferPass::~GBufferPass()
{
    vkDestroyImageView(m_logicalDevice, m_albedo.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_position.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_normal.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_depth.imageView, nullptr);
    vkDestroyFramebuffer(m_logicalDevice, m_framebuffer, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_matrixDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_textureDescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

void GBufferPass::initialize(const fw::Camera* camera)
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    createRenderPass();
    createFramebuffer();
    createDescriptorSetLayouts();
    createPipeline();
    CHECK(m_sampler.create(VK_COMPARE_OP_ALWAYS));
    createDescriptorPool();
    createRenderObjects();

    m_camera = camera;
    m_ubo.proj = m_camera->getProjectionMatrix();
}

void GBufferPass::update()
{
    m_ubo.world = m_trans.getWorldMatrix();
    m_ubo.view = m_camera->getViewMatrix();
    m_uniformBuffer.setData(sizeof(m_ubo), &m_ubo);
}

void GBufferPass::writeRenderCommands(VkCommandBuffer cb)
{
    std::array<VkClearValue, 4> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[3].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.framebuffer = m_framebuffer;

    VkDeviceSize offsets[] = {0};

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    for (const RenderObject& ro : m_renderObjects)
    {
        VkBuffer vb = ro.vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
        vkCmdBindIndexBuffer(cb, ro.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        std::vector<VkDescriptorSet> sets{ro.matrixDescriptorSet, ro.textureDescriptorSet};
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, fw::ui32size(sets), sets.data(), 0, nullptr);
        vkCmdDrawIndexed(cb, ro.numIndices, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(cb);
}

void GBufferPass::createRenderPass()
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference albedoAttachment{};
    albedoAttachment.attachment = 0;
    albedoAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference positionAttachment{};
    positionAttachment.attachment = 1;
    positionAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference normalAttachment{};
    normalAttachment.attachment = 2;
    normalAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentReference> colorAttachments{albedoAttachment, positionAttachment, normalAttachment};

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = fw::ui32size(colorAttachments);
    subpass.pColorAttachments = colorAttachments.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription colorAttachmentDescription = fw::RenderPass::getColorAttachment();
    colorAttachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();

    std::vector<VkAttachmentDescription> attachmentDescriptions = {colorAttachmentDescription, colorAttachmentDescription, colorAttachmentDescription, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = fw::ui32size(attachmentDescriptions);
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
}

void GBufferPass::createFramebuffer()
{
    VkExtent2D extent = fw::API::getSwapChainExtent();
    uint32_t width = extent.width;
    uint32_t height = extent.height;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CHECK(m_albedo.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_albedo.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_albedo.imageView));
    CHECK(m_albedo.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    CHECK(m_position.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_position.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_position.imageView));
    CHECK(m_position.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    CHECK(m_normal.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_normal.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_normal.imageView));
    CHECK(m_normal.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    VkFormat depthFormat = fw::Constants::depthFormat;
    VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CHECK(m_depth.image.create(width, height, depthFormat, 0, depthImageUsage, 1));
    CHECK(m_depth.image.createView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depth.imageView));
    CHECK(m_depth.image.transitLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

    std::vector<VkImageView> attachments{m_albedo.imageView, m_position.imageView, m_normal.imageView, m_depth.imageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = fw::ui32size(attachments);
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffer));
}

void GBufferPass::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding matrixLayoutBinding{};
    matrixLayoutBinding.binding = 0;
    matrixLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matrixLayoutBinding.descriptorCount = 1;
    matrixLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    matrixLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo matrixLayoutInfo{};
    matrixLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    matrixLayoutInfo.bindingCount = 1;
    matrixLayoutInfo.pBindings = &matrixLayoutBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &matrixLayoutInfo, nullptr, &m_matrixDescriptorSetLayout));

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

void GBufferPass::createPipeline()
{
    std::vector<VkDescriptorSetLayout> layouts{m_matrixDescriptorSetLayout, m_textureDescriptorSetLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "object.vert.spv", c_shaderFolder + "object.frag.spv");

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
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates(3, colorBlendAttachmentState);
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(&colorBlendAttachmentState);
    colorBlendState.attachmentCount = 3;
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();

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

void GBufferPass::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 2;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 4;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void GBufferPass::createRenderObjects()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CHECK(m_uniformBuffer.create(sizeof(MatrixUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));

    fw::Model model;
    CHECK(model.loadModel(c_assetsFolder + "attack_droid.obj"));

    fw::Model::Meshes meshes = model.getMeshes();
    uint32_t numMeshes = fw::ui32size(meshes);

    createDescriptorSets(numMeshes);

    m_renderObjects.resize(numMeshes);

    bool success = true;
    for (unsigned int i = 0; i < numMeshes; ++i)
    {
        const fw::Mesh& mesh = meshes[i];
        RenderObject& ro = m_renderObjects[i];

        success = success
            && ro.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            && ro.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        ro.numIndices = fw::ui32size(mesh.indices);

        std::string textureFile = c_assetsFolder + mesh.getFirstTextureOfType(aiTextureType::aiTextureType_DIFFUSE);
        ro.texture.load(textureFile, VK_FORMAT_R8G8B8A8_UNORM);
        updateDescriptorSet(m_matrixDescriptorSets[i], m_textureDescriptorSets[i], ro.texture.getImageView());
        ro.matrixDescriptorSet = m_matrixDescriptorSets[i];
        ro.textureDescriptorSet = m_textureDescriptorSets[i];
    }

    CHECK(success);
}

void GBufferPass::createDescriptorSets(uint32_t setCount)
{
    auto allocateDescriptorSets = [this, setCount](std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout layout) {
        sets.resize(setCount);
        std::vector<VkDescriptorSetLayout> layouts(setCount, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = setCount;
        allocInfo.pSetLayouts = layouts.data();

        VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, sets.data()));
    };

    allocateDescriptorSets(m_matrixDescriptorSets, m_matrixDescriptorSetLayout);
    allocateDescriptorSets(m_textureDescriptorSets, m_textureDescriptorSetLayout);
}

void GBufferPass::updateDescriptorSet(VkDescriptorSet matrixDescriptorSet, VkDescriptorSet textureDescriptorSet, VkImageView imageView)
{
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_uniformBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(MatrixUBO);

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = matrixDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = m_sampler.getSampler();

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = textureDescriptorSet;
    descriptorWrites[1].dstBinding = 0;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}
