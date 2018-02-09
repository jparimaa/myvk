#include "GUI.h"
#include "Context.h"
#include "Common.h"
#include "Command.h"
#include "API.h"

#include <iostream>

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
    ImGui_ImplGlfwVulkan_Shutdown();
    ImGui::DestroyContext();
}

bool GUI::initialize(VkRenderPass renderPass, VkDescriptorPool descriptorPool) const
{
    ImGui::CreateContext();
    
    bool success = true;
    
    ImGui_ImplGlfwVulkan_Init_Data init_data = {};
    init_data.allocator = nullptr;
    init_data.gpu = Context::getPhysicalDevice();
    init_data.device = Context::getLogicalDevice();
    init_data.render_pass = renderPass;
    init_data.pipeline_cache = VK_NULL_HANDLE;
    init_data.descriptor_pool = descriptorPool;
    init_data.check_vk_result = imguiVkResult;
    // Set true and implement callback functions if you want to handle GUI input
    bool installCallbacks = false;
    success = success && ImGui_ImplGlfwVulkan_Init(API::getGLFWwindow(), installCallbacks, &init_data);    

    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();    
    success = success && ImGui_ImplGlfwVulkan_CreateFontsTexture(commandBuffer);
    Command::endSingleTimeCommands(commandBuffer);
    ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();

    return success;
}

void GUI::beginPass() const
{
    ImGui_ImplGlfwVulkan_NewFrame();
}

void GUI::endPass(VkCommandBuffer commandBuffer) const
{
    ImGui_ImplGlfwVulkan_Render(commandBuffer);
}

}  // namespace fw
