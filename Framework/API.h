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
    static VkFormat getSwapChainImageFormat();
    static VkExtent2D getSwapChainExtent();
    static const std::vector<VkFramebuffer>& getSwapChainFramebuffers();
    
    static VkCommandPool getCommandPool();

    static float getTimeSinceStart();
    static float getTimeDelta();

    static void setCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers);

private:
    static Framework* framework;
};

} // namespace fw
