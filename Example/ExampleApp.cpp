#include "ExampleApp.h"
#include "../Framework/RenderPass.h"
#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/Pipeline.h"

#include <array>

ExampleApp::ExampleApp() {
}

ExampleApp::~ExampleApp() {
    vkDestroyRenderPass(fw::Context::getLogicalDevice(), renderPass, nullptr);
}

bool ExampleApp::initialize() {
    bool success = true;
    success = success && createRenderPass();
    success = success && createPipeline();
    return success;
}

void ExampleApp::update() {
}

void ExampleApp::render() {
}

bool ExampleApp::createRenderPass() {
    VkAttachmentDescription colorAttachment = fw::RenderPass::getDefaultColorAttachment();

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = fw::RenderPass::getDefaultDepthAttachment();

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (VkResult r = vkCreateRenderPass(fw::Context::getLogicalDevice(), &renderPassInfo, nullptr, &renderPass);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a render pass", &r);
        return false;
    }
    return true;
}

bool ExampleApp::createPipeline() {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos =
        fw::Pipeline::getDefaultShaderStageInfos("shader_vert.spv", "shader_frag.spv");

    if (shaderStageInfos.empty()) {
        return false;
    }

    for (const auto& info : shaderStageInfos) {
        vkDestroyShaderModule(fw::Context::getLogicalDevice(), info.module, nullptr);
    }
    
    return true;
}
