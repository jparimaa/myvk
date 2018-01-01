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

    static VkFormat getSwapChainImageFormat();
    static VkExtent2D getSwapChainExtent();
    static bool initializeSwapChain(VkRenderPass renderPass);
    
    static VkCommandPool getCommandPool();

private:
    static Framework* framework;
};

} // namespace fw
