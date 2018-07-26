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
    static VkInstance s_instance;
    static VkSurfaceKHR s_surface;
    static VkPhysicalDevice s_physicalDevice;
    static VkDevice s_logicalDevice;
    static VkQueue s_graphicsQueue;
    static VkQueue s_presentQueue;
    static VkPhysicalDeviceProperties* s_physicalDeviceProperties;
};

} // namespace fw
