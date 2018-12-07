#pragma once

#include <vulkan/vulkan.h>

namespace fw
{
class Command
{
public:
    Command() = delete;
    static bool createGraphicsCommandPool(VkCommandPool* commandPool);
    static bool createComputeCommandPool(VkCommandPool* commandPool);
    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace fw
