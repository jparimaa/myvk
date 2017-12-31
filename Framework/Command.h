#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Command
{
public:
    Command() = delete;
    static bool createDefaultCommandPool(VkCommandPool* commandPool);
    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace fw
