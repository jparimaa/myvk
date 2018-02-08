#pragma once

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

    bool initialize(VkRenderPass renderPass, VkDescriptorPool descriptorPool, GLFWwindow* window);
    void render(VkCommandBuffer commandBuffer);

private:
};

} // namespace fw
