#pragma once

#include "Framework.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class API
{
public:
    friend class Framework;

    API() = delete;

    static bool initializeSwapChain(VkRenderPass renderPass);
    static bool initializeGUI(VkDescriptorPool descriptorPool);

    static VkFormat getSwapChainImageFormat();
    static VkExtent2D getSwapChainExtent();
    static const std::vector<VkFramebuffer>& getSwapChainFramebuffers();

    static VkCommandPool getCommandPool();

    static float getTimeSinceStart();
    static float getTimeDelta();

    static bool isKeyPressed(int key);
    static bool isKeyDown(int key);
    static bool isKeyReleased(int key);
    static float getMouseDeltaX();
    static float getMouseDeltaY();

    static void setCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers);

    static GLFWwindow* getGLFWwindow();

private:
    static Framework* s_framework;
};

} // namespace fw
