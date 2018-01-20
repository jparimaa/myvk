#pragma once

#include "Context.h"
#include "Common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>

namespace fw
{

class Buffer
{
public:
    static void copy(Buffer& src, Buffer& dst, VkDeviceSize size);
    
    Buffer() {};
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;
    
    bool create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);    
    void copyToImage(VkImage image, uint32_t width, uint32_t height) const;

    VkBuffer getBuffer() const;

    template <typename T>
    bool setData(VkDeviceSize size, const T* src);
    
    template <typename T>
    bool createForDevice(const std::vector<T>& content, VkBufferUsageFlagBits flag);

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;    
};

template <typename T>
bool Buffer::setData(VkDeviceSize size, const T* src)
{
    void* dst;
    if (VkResult r = vkMapMemory(logicalDevice, memory, 0, size, 0, &dst);
        r != VK_SUCCESS) {
        printError("Failed to map memory for image");
        return false;
    }
    std::memcpy(dst, src, static_cast<size_t>(size));
    vkUnmapMemory(logicalDevice, memory);
    return true;
}

template <typename T>
bool Buffer::createForDevice(const std::vector<T>& content, VkBufferUsageFlagBits flag)
{
    VkDeviceSize bufferSize = sizeof(content[0]) * content.size();

    Buffer staging;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!staging.create(bufferSize, usage, properties)) {
        return false;
    }

    if (!staging.setData<T>(bufferSize, content.data())) {
        return false;
    }

    if (!create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | flag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        return false;
    }

    copy(staging, *this, bufferSize);
    return true;
}

} // namespace fw
