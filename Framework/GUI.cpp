#include "GUI.h"
#include "Context.h"
#include "Common.h"
#include "Command.h"
#include "RenderPass.h"
#include "API.h"

namespace fw
{

namespace
{

static void imguiVkResult(VkResult r)
{
    if (r != VK_SUCCESS) {
        printError("ImGUI Error: ", &r);
    }
}

} // unnamed

GUI::~GUI()
{
    if (initialized) {
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
        ImGui_ImplGlfwVulkan_Shutdown();
        ImGui::DestroyContext();
    }
}

bool GUI::initialize(VkDescriptorPool descriptorPool)
{
    logicalDevice = Context::getLogicalDevice();
    
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    bool success = createCommandBuffer() && createRenderPass();    
    
    ImGui::CreateContext();
    
    ImGui_ImplGlfwVulkan_Init_Data initData = {};
    initData.allocator = nullptr;
    initData.gpu = Context::getPhysicalDevice();
    initData.device = logicalDevice;
    initData.render_pass = renderPass;
    initData.pipeline_cache = VK_NULL_HANDLE;
    initData.descriptor_pool = descriptorPool;
    initData.check_vk_result = imguiVkResult;
    bool installCallbacks = false;
    success = success && ImGui_ImplGlfwVulkan_Init(API::getGLFWwindow(), installCallbacks, &initData);    

    VkCommandBuffer singleTimeCommandBuffer = Command::beginSingleTimeCommands();    
    success = success && ImGui_ImplGlfwVulkan_CreateFontsTexture(singleTimeCommandBuffer);
    Command::endSingleTimeCommands(singleTimeCommandBuffer);
    ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();

    initialized = true;
    
    return success;
}

void GUI::beginPass() const
{   
    ImGui_ImplGlfwVulkan_NewFrame();
}

bool GUI::render(VkFramebuffer framebuffer) const
{
    vkBeginCommandBuffer(commandBuffer, &info);
        
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.extent = API::getSwapChainExtent();
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplGlfwVulkan_Render(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);
    if (VkResult r = vkEndCommandBuffer(commandBuffer);
        r != VK_SUCCESS) {
        fw::printError("Failed to record GUI command buffer", &r);
        return false;
    }
    return true;
}

VkCommandBuffer GUI::getCommandBuffer() const
{
    return commandBuffer;
}

bool GUI::isInitialized() const
{
    return initialized;
}

bool GUI::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = fw::API::getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (VkResult r = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
        r != VK_SUCCESS) {
        fw::printError("Failed to allocate GUI command buffer", &r);
        return false;
    }
    return true;
}

bool GUI::createRenderPass()
{
    VkAttachmentDescription colorAttachment = fw::RenderPass::getColorAttachment();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

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
    renderPassInfo.attachmentCount = fw::ui32size(attachments);
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (VkResult r = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a GUI render pass", &r);
        return false;
    }
    return true;   
}   

}  // namespace fw
