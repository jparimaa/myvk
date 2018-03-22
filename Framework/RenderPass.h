#pragma once

#include <vulkan/vulkan.h>

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
