#include "GUI.h"
#include "Context.h"
#include "Common.h"
#include "Command.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_vulkan.h"

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

bool GUI::initialize(VkRenderPass renderPass, VkDescriptorPool descriptorPool, GLFWwindow* window)
{
    ImGui::CreateContext();    
    ImGui_ImplGlfwVulkan_Init_Data init_data = {};
    init_data.allocator = nullptr;
    init_data.gpu = Context::getPhysicalDevice();
    init_data.device = Context::getLogicalDevice();
    init_data.render_pass = renderPass;
    init_data.pipeline_cache = VK_NULL_HANDLE;
    init_data.descriptor_pool = descriptorPool;
    init_data.check_vk_result = imguiVkResult;
    bool success = true;
    success = success && ImGui_ImplGlfwVulkan_Init(window, true, &init_data);

    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();    
    success = success && ImGui_ImplGlfwVulkan_CreateFontsTexture(commandBuffer);
    Command::endSingleTimeCommands(commandBuffer);
    ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
    
    return success;
}

void GUI::render(VkCommandBuffer commandBuffer)
{
    ImGui_ImplGlfwVulkan_NewFrame();
    ImGui::Text("Hello, world!");
    ImGui_ImplGlfwVulkan_Render(commandBuffer);
}

}  // namespace fw
