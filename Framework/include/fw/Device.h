#pragma once

#include <vulkan/vulkan.h>

namespace fw
{

class Device
{
public:
    Device() {};
    ~Device();
    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

    bool initialize();

private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physicalDeviceProperties;

    bool getPhysicalDevice();
    bool createLogicalDevice();
};

} // namespace fw
