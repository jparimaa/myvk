#include "ReflectionApp.h"
#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Constants.h"
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
const int c_inputTextureCount = 3;
} // unnamed

ReflectionApp::~ReflectionApp()
{
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_textureDescriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool ReflectionApp::initialize()
{
    m_gbufferPass.initialize(&m_camera);

    m_logicalDevice = fw::Context::getLogicalDevice();

    createRenderPass();
    CHECK(fw::API::initializeSwapChainWithDefaultFramebuffer(m_renderPass));
    createDescriptorSetLayouts();
    createPipeline();
    CHECK(m_sampler.create(VK_COMPARE_OP_ALWAYS));
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    m_cameraController.setCamera(&m_camera);
    m_cameraController.setMovementSpeed(20.0f);
    glm::vec3 initPos(0.0f, 10.0f, 40.0f);
    m_cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    m_camera.setPosition(initPos);

    return true;
}

void ReflectionApp::update()
{
    m_cameraController.update();
    m_gbufferPass.update();
}

void ReflectionApp::createRenderPass()
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

void ReflectionApp::createDescriptorSetLayouts()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings(c_inputTextureCount);

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

    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = fw::ui32size(bindings);
    textureLayoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &textureLayoutInfo, nullptr, &m_textureDescriptorSetLayout));
}

void ReflectionApp::createPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> layouts{m_textureDescriptorSetLayout};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "reflection.vert.spv", c_shaderFolder + "reflection.frag.spv");

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

void ReflectionApp::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 16;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 16;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void ReflectionApp::createDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_textureDescriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_textureDescriptorSet));

    std::vector<VkImageView> imageViews{
        m_gbufferPass.getAlbedoImageView(),
        m_gbufferPass.getPositionImageView(),
        m_gbufferPass.getNormalImageView()};

    std::vector<VkWriteDescriptorSet> descriptorWrites(c_inputTextureCount);
    std::vector<VkDescriptorImageInfo> imageInfos(c_inputTextureCount);

    for (int i = 0; i < c_inputTextureCount; ++i)
    {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = imageViews[i];
        imageInfos[i].sampler = m_sampler.getSampler();

        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_textureDescriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pImageInfo = &imageInfos[i];
    }

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void ReflectionApp::createCommandBuffers()
{
    const std::vector<VkFramebuffer>& swapChainFramebuffers = fw::API::getSwapChainFramebuffers();
    std::vector<VkCommandBuffer> commandBuffers(fw::API::getSwapChainImageCount());

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
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();

    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        VkCommandBuffer cb = commandBuffers[i];
        VK_CHECK(vkBeginCommandBuffer(cb, &beginInfo));

        m_gbufferPass.writeRenderCommands(cb);

        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_textureDescriptorSet, 0, nullptr);
        vkCmdDraw(cb, 3, 1, 0, 0);

        vkCmdEndRenderPass(cb);

        VK_CHECK(vkEndCommandBuffer(cb));
    }

    fw::API::setCommandBuffers(commandBuffers);
}