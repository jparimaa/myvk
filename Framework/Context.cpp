#include "Context.h"

namespace fw {

VkInstance Context::instance = VK_NULL_HANDLE;

VkInstance Context::getInstance() {
    return instance;
}

} // namespace fw
