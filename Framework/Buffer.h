#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Buffer
{
public:
    Buffer() = delete;
    static bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkBuffer* buffer, VkDeviceMemory* bufferMemory);
    static void copy(VkBuffer src, VkBuffer dst, VkDeviceSize size);
};

} // namespace fw
