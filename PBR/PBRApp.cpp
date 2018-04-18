#include "PBRApp.h"
#include "../Framework/RenderPass.h"
#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/Pipeline.h"
#include "../Framework/Command.h"
#include "../Framework/API.h"
#include "../Framework/Model.h"
#include "../Framework/Mesh.h"

#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <array>

PBRApp::~PBRApp()
{
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}

bool PBRApp::initialize()
{
    logicalDevice = fw::Context::getLogicalDevice();
    bool success =
        environmentImages.initialize(assetsFolder + "Factory_Catwalk_2k.hdr") &&
        brdfLut.initialize() &&
        createRenderPass() &&
        fw::API::initializeSwapChain(renderPass) &&
        sampler.create(VK_COMPARE_OP_ALWAYS) &&
        createDescriptorPool() &&
        createRenderObject() &&
        fw::API::initializeGUI(descriptorPool) &&
        skybox.initialize(renderPass, descriptorPool, sampler.getSampler(), environmentImages.getPlainImageView()) &&
        createCommandBuffers();

    extent = fw::API::getSwapChainExtent();
    cameraController.setCamera(&camera);
    glm::vec3 initPos(0.0f, 0.0f, 0.0f);
    cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    camera.setPosition(initPos);

    return success;
}

void PBRApp::update()
{
    cameraController.update();
    skybox.update(camera);
}

void PBRApp::onGUI()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"

    glm::vec3 p = camera.getTransformation().getPosition();
    ImGui::Text("Camera position: %.1f %.1f %.1f", p.x, p.y, p.z);
    ImGui::Text("%.2f ms/frame (%.0f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

#pragma GCC diagnostic pop
}

bool PBRApp::createRenderPass()
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

    if (VkResult r = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a render pass", &r);
        return false;
    }
    return true;
}

bool PBRApp::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;  // Optional

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

    if (VkResult r = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout);
        r != VK_SUCCESS) {
        fw::printError("Failed to create descriptor set layout", &r);
        return false;
    }
    return true;
}

bool PBRApp::createDescriptorPool()
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

    if (VkResult r = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool);
        r != VK_SUCCESS) {
        fw::printError("Failed to create descriptor pool", &r);
        return false;
    }
    return true;
}

bool PBRApp::createRenderObject()
{
    fw::Model model;
    if (!model.loadModel(assetsFolder + "DamagedHelmet.gltf")) {
        return false;
    }

    const std::vector<unsigned char>& data = model.getTextureData(0);
    renderObject.texture.load(data.data(), data.size());

    fw::Model::Meshes meshes = model.getMeshes();
    if (meshes.size() != 1) {
        fw::printError("Expected that render object has only one mesh");
        return false;
    }

    const fw::Mesh& mesh = meshes[0];

    bool success =
        renderObject.vertexBuffer.createForDevice<glm::vec3>(mesh.positions, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) &&
        renderObject.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    renderObject.numIndices = mesh.indices.size();

    return success;
}

bool PBRApp::createCommandBuffers()
{
    const std::vector<VkFramebuffer>& swapChainFramebuffers = fw::API::getSwapChainFramebuffers();
    std::vector<VkCommandBuffer> commandBuffers(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = fw::API::getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = fw::ui32size(commandBuffers);

    if (VkResult r = vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data());
        r != VK_SUCCESS) {
        fw::printError("Failed to allocate command buffers", &r);
        return false;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.2f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();

    for (size_t i = 0; i < commandBuffers.size(); ++i) {
        VkCommandBuffer cb = commandBuffers[i];

        vkBeginCommandBuffer(cb, &beginInfo);

        renderPassInfo.framebuffer = swapChainFramebuffers[i];

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        skybox.render(cb);
        vkCmdEndRenderPass(cb);

        if (VkResult r = vkEndCommandBuffer(cb);
            r != VK_SUCCESS) {
            fw::printError("Failed to record command buffer", &r);
            return false;
        }
    }

    fw::API::setCommandBuffers(commandBuffers);
    return true;
}
