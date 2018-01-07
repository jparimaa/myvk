#pragma once

#include "Context.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>

namespace fw
{

class Buffer
{
public:
    struct AutoBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        ~AutoBuffer();
    };
    
    Buffer() = delete;
    static bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       VkBuffer* buffer, VkDeviceMemory* bufferMemory);
    static bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                       AutoBuffer& staging);
    static void copy(VkBuffer src, VkBuffer dst, VkDeviceSize size);

    template <typename T>
    static bool createForDevice(const std::vector<T>& content, VkBufferUsageFlagBits flag, AutoBuffer& buffer)
        {
            VkDevice logicalDevice = Context::getLogicalDevice();
            VkDeviceSize bufferSize = sizeof(content[0]) * content.size();

            AutoBuffer staging;
            if (!create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        staging)) {
                return false;
            }

            void* data;
            vkMapMemory(logicalDevice, staging.memory, 0, bufferSize, 0, &data);
            std::memcpy(data, content.data(), (size_t)bufferSize);
            vkUnmapMemory(logicalDevice, staging.memory);

            if (!create(bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | flag,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        buffer)) {
                return false;
            }

            copy(staging.buffer, buffer.buffer, bufferSize);
            return true;
        }
};

} // namespace fw
