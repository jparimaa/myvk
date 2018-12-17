#include "ParticlesApp.h"
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
#include <vulkan/vulkan.h>

#include <array>
#include <iostream>

ParticlesApp::~ParticlesApp()
{
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool ParticlesApp::initialize()
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    VkPhysicalDeviceProperties* physicalDeviceProperties = fw::Context::getPhysicalDeviceProperties();
    VkPhysicalDeviceLimits limits = physicalDeviceProperties->limits;
    uint32_t maxComputeWorkGroupCount[3] = {
        limits.maxComputeWorkGroupCount[0],
        limits.maxComputeWorkGroupCount[1],
        limits.maxComputeWorkGroupCount[2]};
    uint32_t maxComputeWorkGroupSize[3] = {
        limits.maxComputeWorkGroupSize[0],
        limits.maxComputeWorkGroupSize[1],
        limits.maxComputeWorkGroupSize[2]};
    uint32_t maxComputeWorkGroupInvocations = limits.maxComputeWorkGroupInvocations;

    std::cout << "maxComputeWorkGroupCount[0]: " << maxComputeWorkGroupCount[0] << "\n"
              << "maxComputeWorkGroupCount[1]: " << maxComputeWorkGroupCount[1] << "\n"
              << "maxComputeWorkGroupCount[2]: " << maxComputeWorkGroupCount[2] << "\n"
              << "maxComputeWorkGroupSize[0]: " << maxComputeWorkGroupSize[0] << "\n"
              << "maxComputeWorkGroupSize[1]: " << maxComputeWorkGroupSize[1] << "\n"
              << "maxComputeWorkGroupSize[2]: " << maxComputeWorkGroupSize[2] << "\n"
              << "maxComputeWorkGroupInvocations: " << maxComputeWorkGroupInvocations << "\n";

    createBuffer();
    m_particleCompute.initialize(&m_storageBuffer);
    createRenderPass();
    bool success = fw::API::initializeSwapChainWithDefaultFramebuffer(m_renderPass);
    createDescriptorSetLayout();
    createPipeline();
    success = success && m_sampler.create(VK_COMPARE_OP_ALWAYS);
    createDescriptorPool();
    createRenderObjects();
    success = success && fw::API::initializeGUI(m_descriptorPool);
    createCommandBuffers();

    CHECK(success);

    m_cameraController.setCamera(&m_camera);
    glm::vec3 initPos(0.0f, 0.0f, 15.0f);
    m_cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    m_cameraController.setMovementSpeed(50.0f);
    m_camera.setPosition(initPos);
    m_camera.setNearClipDistance(3.0f);
    m_camera.setFarClipDistance(2000.0f);

    m_matrices.proj = m_camera.getProjectionMatrix();

    return true;
}

void ParticlesApp::update()
{
    m_transformation.rotateUp(fw::API::getTimeDelta() * glm::radians(45.0f));
    m_matrices.world = m_transformation.getWorldMatrix();

    m_cameraController.update();
    m_matrices.view = m_camera.getViewMatrix();

    m_uniformBuffer.setData(sizeof(m_matrices), &m_matrices);
}

void ParticlesApp::onGUI()
{
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

    glm::vec3 p = m_camera.getTransformation().getPosition();
    ImGui::Text("Camera position: %.1f %.1f %.1f", p.x, p.y, p.z);
    ImGui::Text("%.2f ms/frame (%.0f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

#ifndef WIN32
#pragma GCC diagnostic pop
#endif
}

void ParticlesApp::createBuffer()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    m_storageBuffer.create(c_bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, uboProperties);
}

void ParticlesApp::createRenderPass()
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

void ParticlesApp::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = fw::ui32size(bindings);
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void ParticlesApp::createPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_descriptorSetLayout);

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "shader.vert.spv", c_shaderFolder + "shader.frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
        for (const auto& info : shaderStages)
        {
            vkDestroyShaderModule(m_logicalDevice, info.module, nullptr);
        }
    });

    VkVertexInputBindingDescription vertexDescription{};
    vertexDescription.binding = 0;
    vertexDescription.stride = sizeof(Particle);
    vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Particle, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Particle, direction);

    VkPipelineVertexInputStateCreateInfo vertexInputState = fw::Pipeline::getVertexInputState(&vertexDescription, &attributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = fw::Pipeline::getInputAssemblyState();
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

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

void ParticlesApp::createDescriptorPool()
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

void ParticlesApp::createRenderObjects()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CHECK(m_uniformBuffer.create(c_transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));

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
        updateDescriptorSet(m_descriptorSets[i], ro.texture.getImageView());
        ro.descriptorSet = m_descriptorSets[i];
    }

    CHECK(success);
}

void ParticlesApp::createDescriptorSets(uint32_t setCount)
{
    m_descriptorSets.resize(setCount);

    std::vector<VkDescriptorSetLayout> layouts(setCount, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()));
}

void ParticlesApp::updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView)
{
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_uniformBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = c_transformMatricesSize;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
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
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void ParticlesApp::createCommandBuffers()
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
    beginInfo.pInheritanceInfo = nullptr; // Optional

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

    VkDeviceSize offsets[] = {0};

    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        VkCommandBuffer cb = commandBuffers[i];

        vkBeginCommandBuffer(cb, &beginInfo);

        renderPassInfo.framebuffer = swapChainFramebuffers[i];

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        for (const RenderObject& ro : m_renderObjects)
        {
            VkBuffer vb = m_storageBuffer.getBuffer();
            vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &ro.descriptorSet, 0, nullptr);
            vkCmdDraw(cb, c_numParticles, 1, 0, 0);
        }

        vkCmdEndRenderPass(cb);

        VK_CHECK(vkEndCommandBuffer(cb));
    }

    fw::API::setCommandBuffers(commandBuffers);
}
