#include "Context.h"

namespace fw
{

VkInstance Context::instance = VK_NULL_HANDLE;
VkSurfaceKHR Context::surface = VK_NULL_HANDLE;
VkPhysicalDevice Context::physicalDevice = VK_NULL_HANDLE;
VkDevice Context::logicalDevice = VK_NULL_HANDLE;

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

} // namespace fw
