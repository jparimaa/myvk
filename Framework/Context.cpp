#include "Context.h"

namespace fw
{

VkInstance Context::instance = VK_NULL_HANDLE;
VkSurfaceKHR Context::surface = VK_NULL_HANDLE;
VkPhysicalDevice Context::physicalDevice = VK_NULL_HANDLE;
VkDevice Context::logicalDevice = VK_NULL_HANDLE;
VkQueue Context::graphicsQueue = VK_NULL_HANDLE;
VkQueue Context::presentQueue = VK_NULL_HANDLE;
VkPhysicalDeviceProperties* Context::physicalDeviceProperties = nullptr;

VkInstance Context::getInstance()
{
    return instance;
}

VkSurfaceKHR Context::getSurface()
{
    return surface;
}

VkPhysicalDevice Context::getPhysicalDevice()
{
    return physicalDevice;
}

VkDevice Context::getLogicalDevice()
{
    return logicalDevice;
}

VkQueue Context::getGraphicsQueue()
{
    return graphicsQueue;
}

VkQueue Context::getPresentQueue()
{
    return presentQueue;
}

VkPhysicalDeviceProperties* Context::getPhysicalDeviceProperties()
{
    return physicalDeviceProperties;
}

} // namespace fw
