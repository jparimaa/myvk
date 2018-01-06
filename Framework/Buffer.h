#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Buffer
{
public:
    struct Staging {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        ~Staging();
    };
    
    Buffer() = delete;
    static bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkBuffer* buffer, VkDeviceMemory* bufferMemory);
    static bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       Staging& staging);
    static void copy(VkBuffer src, VkBuffer dst, VkDeviceSize size);
};

} // namespace fw
