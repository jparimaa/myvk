// This class is meant only for Vulkan handles

#pragma once

#include <vulkan/vulkan.h>

namespace fw
{

class Context
{
public:
    friend class Instance;
    friend class Window;
    friend class Device;

    Context() = delete;
    static VkInstance getInstance();
    static VkSurfaceKHR getSurface();
    static VkPhysicalDevice getPhysicalDevice();
    static VkDevice getLogicalDevice();
    static VkQueue getGraphicsQueue();
    static VkQueue getPresentQueue();
    static VkPhysicalDeviceProperties* getPhysicalDeviceProperties();

private:
    static VkInstance instance;
    static VkSurfaceKHR surface;
    static VkPhysicalDevice physicalDevice;
    static VkDevice logicalDevice;
    static VkQueue graphicsQueue;
    static VkQueue presentQueue;
    static VkPhysicalDeviceProperties* physicalDeviceProperties;
};

} // namespace fw
