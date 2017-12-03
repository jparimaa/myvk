#include "Context.h"

namespace fw {

VkInstance Context::instance = VK_NULL_HANDLE;
VkSurfaceKHR Context::surface = VK_NULL_HANDLE;

VkInstance Context::getInstance() {
    return instance;
}

VkSurfaceKHR Context::getSurface() {
    return surface;
}

} // namespace fw
