#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Sampler
{
public:
    Sampler();
    ~Sampler();
    bool create();

private:    
    VkSampler textureSampler = VK_NULL_HANDLE;
};

} // namespace fw
