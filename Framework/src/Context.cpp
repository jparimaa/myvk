#include "Context.h"

namespace fw
{

VkInstance Context::s_instance = VK_NULL_HANDLE;
VkSurfaceKHR Context::s_surface = VK_NULL_HANDLE;
VkPhysicalDevice Context::s_physicalDevice = VK_NULL_HANDLE;
VkDevice Context::s_logicalDevice = VK_NULL_HANDLE;
VkQueue Context::s_graphicsQueue = VK_NULL_HANDLE;
VkQueue Context::s_presentQueue = VK_NULL_HANDLE;
VkPhysicalDeviceProperties* Context::s_physicalDeviceProperties = nullptr;

VkInstance Context::getInstance()
{
    return s_instance;
}

VkSurfaceKHR Context::getSurface()
{
    return s_surface;
}

VkPhysicalDevice Context::getPhysicalDevice()
{
    return s_physicalDevice;
}

VkDevice Context::getLogicalDevice()
{
    return s_logicalDevice;
}

VkQueue Context::getGraphicsQueue()
{
    return s_graphicsQueue;
}

VkQueue Context::getPresentQueue()
{
    return s_presentQueue;
}

VkPhysicalDeviceProperties* Context::getPhysicalDeviceProperties()
{
    return s_physicalDeviceProperties;
}

} // namespace fw
