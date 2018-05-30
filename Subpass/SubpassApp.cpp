#include "SubpassApp.h"
#include "../Framework/RenderPass.h"
#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/Pipeline.h"
#include "../Framework/Command.h"
#include "../Framework/API.h"
#include "../Framework/Model.h"
#include "../Framework/Mesh.h"
#include "../Framework/Macros.h"

#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <array>

namespace
{

const std::size_t c_transformMatricesSize = sizeof(glm::mat4x4) * 3;
const std::string c_assetsFolder = "../Assets/";
const uint32_t c_gbufferTextureCount = 3;

} // unnamed

SubpassApp::~SubpassApp()
{
    auto destroySubpass = [this](Subpass& subpass) {
        vkDestroyPipeline(m_logicalDevice, subpass.pipeline, nullptr);
        vkDestroyPipelineLayout(m_logicalDevice, subpass.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_logicalDevice, subpass.descriptorSetLayout, nullptr);
    };

    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    destroySubpass(m_gbuffer);
    destroySubpass(m_composite);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool SubpassApp::initialize()
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    createRenderPass();
    createFramebuffers();
    createDescriptorSetLayouts();
    createGBufferPipeline();
    createCompositePipeline();
    bool success = m_sampler.create(VK_COMPARE_OP_ALWAYS);
    createDescriptorPool();
    createGBufferAttachments();
    createRenderObjects();
    createAndUpdateCompositeDescriptorSet();
    success = success && fw::API::initializeGUI(m_descriptorPool);
    createCommandBuffers();

    CHECK(success);

    m_cameraController.setCamera(&m_camera);
    glm::vec3 initPos(0.0f, 10.0f, 40.0f);
    m_cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    m_camera.setPosition(initPos);

    m_ubo.proj = m_camera.getProjectionMatrix();

    return true;
}

void SubpassApp::update()
{
    m_transformation.rotateUp(fw::API::getTimeDelta() * glm::radians(45.0f));
    m_ubo.world = m_transformation.getWorldMatrix();

    m_cameraController.update();
    m_ubo.view = m_camera.getViewMatrix();

    m_uniformBuffer.setData(sizeof(m_ubo), &m_ubo);
}

void SubpassApp::createRenderPass()
{
    VkAttachmentDescription colorAttachment = fw::RenderPass::getColorAttachment();

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

void SubpassApp::createFramebuffers()
{
    const uint32_t attachmentCount = c_gbufferTextureCount + 2; // +Depth & final composite
    VkImageView attachments[attachmentCount];

    VkExtent2D extent = fw::API::getSwapChainExtent();

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = attachmentCount;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;

    m_framebuffers.resize(fw::API::getSwapChainImageCount());
    const std::vector<VkImageView>& swapChainImageViews = fw::API::getSwapChainImageViews();
    const VkImageView& depthImageView = fw::API::getSwapChainDepthImageView();

    for (unsigned int i = 0; i < m_framebuffers.size(); ++i) {
        attachments[0] = swapChainImageViews[i];
        attachments[1] = m_framebufferAttachments[0].imageView;
        attachments[2] = m_framebufferAttachments[1].imageView;
        attachments[3] = m_framebufferAttachments[2].imageView;
        attachments[4] = depthImageView;
        VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferCreateInfo, nullptr, &m_framebuffers[i]));
    }
}

void SubpassApp::createDescriptorSetLayouts()
{
    auto createDescriptorSetLayout = [this](const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout& descriptorSetLayout) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = fw::ui32size(bindings);
        layoutInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout));
    };

    // GBuffer
    VkDescriptorSetLayoutBinding matrixBinding{};
    matrixBinding.binding = 0;
    matrixBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matrixBinding.descriptorCount = 1;
    matrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    matrixBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> gbufferBindings = {matrixBinding, samplerBinding};
    createDescriptorSetLayout(gbufferBindings, m_gbuffer.descriptorSetLayout);

    // Composite
    VkDescriptorSetLayoutBinding positionSamplerBinding{};
    positionSamplerBinding.binding = 0;
    positionSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    positionSamplerBinding.descriptorCount = 1;
    positionSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    positionSamplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalSamplerBinding{};
    normalSamplerBinding.binding = 1;
    normalSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    normalSamplerBinding.descriptorCount = 1;
    normalSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding albedoSamplerBinding{};
    albedoSamplerBinding.binding = 2;
    albedoSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    albedoSamplerBinding.descriptorCount = 1;
    albedoSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoSamplerBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> compositeBindings = {positionSamplerBinding, normalSamplerBinding, albedoSamplerBinding};
    createDescriptorSetLayout(compositeBindings, m_composite.descriptorSetLayout);
}

void SubpassApp::createGBufferPipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = fw::Pipeline::getShaderStageInfos("gbuffer_vert.spv", "gbuffer_frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
            for (const auto& info : shaderStages) {
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
    VkPipelineLayoutCreateInfo m_pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_gbuffer.descriptorSetLayout);
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(c_gbufferTextureCount, colorBlendAttachmentState);
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(nullptr);
    colorBlendState.pAttachments = blendAttachmentStates.data();
    colorBlendState.attachmentCount = c_gbufferTextureCount;

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &m_pipelineLayoutInfo, nullptr, &m_gbuffer.pipelineLayout));

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
    pipelineInfo.layout = m_gbuffer.pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_gbuffer.pipeline));
}

void SubpassApp::createCompositePipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = fw::Pipeline::getShaderStageInfos("composite_vert.spv", "composite_frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
            for (const auto& info : shaderStages) {
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
    VkPipelineLayoutCreateInfo m_pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_composite.descriptorSetLayout);

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &m_pipelineLayoutInfo, nullptr, &m_composite.pipelineLayout));

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
    pipelineInfo.layout = m_gbuffer.pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_composite.pipeline));
}

void SubpassApp::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 2;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void SubpassApp::createGBufferAttachments()
{
    auto createAttachment = [this](VkFormat format, FramebufferAttachment& attachment) {
        VkExtent2D extent = fw::API::getSwapChainExtent();
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        attachment.image.create(extent.width, extent.height, format, 0, usage);
        attachment.image.createView(format, VK_IMAGE_ASPECT_COLOR_BIT, &attachment.imageView);
    };

    m_framebufferAttachments.resize(c_gbufferTextureCount);

    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, m_framebufferAttachments[0]); // Positions
    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, m_framebufferAttachments[1]); // Normals
    createAttachment(VK_FORMAT_R8G8B8A8_UNORM, m_framebufferAttachments[2]); // Albedo
}

void SubpassApp::createRenderObjects()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CHECK(m_uniformBuffer.create(c_transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));

    fw::Model model;
    CHECK(model.loadModel(c_assetsFolder + "attack_droid.obj"));

    fw::Model::Meshes meshes = model.getMeshes();
    uint32_t numMeshes = fw::ui32size(meshes);

    createGBufferDescriptorSets(numMeshes);

    m_renderObjects.resize(numMeshes);

    bool success = true;
    for (unsigned int i = 0; i < numMeshes; ++i) {
        const fw::Mesh& mesh = meshes[i];
        RenderObject& ro = m_renderObjects[i];

        success = success &&
            ro.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)  &&
            ro.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        ro.numIndices = mesh.indices.size();

        std::string textureFile = c_assetsFolder + mesh.getFirstTextureOfType(aiTextureType::aiTextureType_DIFFUSE);
        ro.texture.load(textureFile, VK_FORMAT_R8G8B8A8_UNORM);
        updateGBufferDescriptorSet(m_gbuffer.descriptorSets[i], ro.texture.getImageView());
        ro.descriptorSet = m_gbuffer.descriptorSets[i];
    }

    CHECK(success);
}

void SubpassApp::createGBufferDescriptorSets(uint32_t setCount)
{
    m_gbuffer.descriptorSets.resize(setCount);

    std::vector<VkDescriptorSetLayout> gbufferLayouts(setCount, m_gbuffer.descriptorSetLayout);
    VkDescriptorSetAllocateInfo gbufferAllocInfo{};
    gbufferAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    gbufferAllocInfo.descriptorPool = m_descriptorPool;
    gbufferAllocInfo.descriptorSetCount = setCount;
    gbufferAllocInfo.pSetLayouts = gbufferLayouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &gbufferAllocInfo, m_gbuffer.descriptorSets.data()));
}

void SubpassApp::updateGBufferDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView)
{
    VkDescriptorBufferInfo matrixBufferInfo{};
    matrixBufferInfo.buffer = m_uniformBuffer.getBuffer();
    matrixBufferInfo.offset = 0;
    matrixBufferInfo.range = c_transformMatricesSize;


    VkDescriptorImageInfo albedoTextureInfo{};
    albedoTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    albedoTextureInfo.imageView = imageView;
    albedoTextureInfo.sampler = m_sampler.getSampler();

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &matrixBufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &albedoTextureInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void SubpassApp::createAndUpdateCompositeDescriptorSet()
{
    std::vector<VkDescriptorSetLayout> compositeLayouts(1, m_composite.descriptorSetLayout);
    VkDescriptorSetAllocateInfo compositeAllocInfo{};
    compositeAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    compositeAllocInfo.descriptorPool = m_descriptorPool;
    compositeAllocInfo.descriptorSetCount = fw::ui32size(compositeLayouts);
    compositeAllocInfo.pSetLayouts = compositeLayouts.data();

    m_composite.descriptorSets.resize(1);

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &compositeAllocInfo, m_composite.descriptorSets.data()));

    unsigned int numAttachments = m_framebufferAttachments.size();
    std::vector<VkWriteDescriptorSet> descriptorWrites(numAttachments);

    for (unsigned int i = 0; i < numAttachments; ++i) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_framebufferAttachments[i].imageView;
        imageInfo.sampler = m_sampler.getSampler();

        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_composite.descriptorSets[i];
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void SubpassApp::createCommandBuffers()
{
    const std::vector<VkFramebuffer>& swapChainFramebuffers = fw::API::getSwapChainFramebuffers();
    std::vector<VkCommandBuffer> commandBuffers(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = fw::API::getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = fw::ui32size(commandBuffers);

    VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, commandBuffers.data()));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;  // Optional

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

    for (size_t i = 0; i < commandBuffers.size(); ++i) {
        VkCommandBuffer cb = commandBuffers[i];

        vkBeginCommandBuffer(cb, &beginInfo);

        renderPassInfo.framebuffer = swapChainFramebuffers[i];

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbuffer.pipeline);

        for (const RenderObject& ro : m_renderObjects) {
            VkBuffer vb = ro.vertexBuffer.getBuffer();
            vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
            vkCmdBindIndexBuffer(cb, ro.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbuffer.pipelineLayout, 0, 1, &ro.descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(cb, ro.numIndices, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(cb);

        VK_CHECK(vkEndCommandBuffer(cb));
    }

    fw::API::setCommandBuffers(commandBuffers);
}
