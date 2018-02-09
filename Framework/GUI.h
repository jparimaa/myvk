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

    bool initialize(VkRenderPass renderPass, VkDescriptorPool descriptorPool) const;
    void beginPass() const;
    void endPass(VkCommandBuffer commandBuffer) const;

private:
};

} // namespace fw
