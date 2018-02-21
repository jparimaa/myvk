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
    Sampler(const Sampler&) = delete;
    Sampler(Sampler&&) = delete;
    Sampler& operator=(const Sampler&) = delete;
    Sampler& operator=(Sampler&&) = delete;
    
    bool create(VkCompareOp compareOp);

    VkSampler getSampler() const;

private:    
    VkSampler sampler = VK_NULL_HANDLE;
};

} // namespace fw
