#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class RenderPass
{
public:    
    RenderPass() = delete;
    static VkAttachmentDescription getColorAttachment();
    static VkAttachmentDescription getDepthAttachment();
};

} // namespace fw
