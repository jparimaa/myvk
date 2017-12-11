#pragma once

#include "Framework.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class API {
public:
    friend class Framework;
    
    API() = delete;

    static VkFormat getSwapChainImageFormat();

private:
    static Framework* framework;
};

} // namespace fw