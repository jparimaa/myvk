#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class GUI
{
public:
    GUI() {};
    ~GUI();
    GUI(const GUI&) = delete;
    GUI(GUI&&) = delete;
    GUI& operator=(const GUI&) = delete;
    GUI& operator=(GUI&&) = delete;

    bool initialize(VkDescriptorPool descriptorPool);
    void beginPass() const;
    bool render(VkFramebuffer framebuffer) const;
    VkCommandBuffer getCommandBuffer() const;

    bool isInitialized() const;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkCommandBufferBeginInfo info = {};
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    bool initialized = false;

    bool createCommandBuffer();
    bool createRenderPass();
};

} // namespace fw