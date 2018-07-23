#pragma once

#ifndef WIN32
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_vulkan.h"
#ifndef WIN32
	#pragma GCC diagnostic pop
#endif

#include <vulkan/vulkan.h>

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
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkCommandBufferBeginInfo m_info{};
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    bool m_initialized = false;

    bool createCommandBuffer();
    bool createRenderPass();
};

} // namespace fw
