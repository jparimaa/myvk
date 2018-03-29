#pragma once

#include "../Framework/Image.h"

#include <vulkan/vulkan.h>

class Offscreen
{
public:
    static VkRenderPass renderPass;
    static VkFormat format;

    Offscreen() {};
    ~Offscreen();
    bool createFramebuffer(uint32_t size, uint32_t layerCount, uint32_t levelCount);

    uint32_t getSize() const;
    VkImage getImageHandle() const;
    VkFramebuffer getFramebuffer() const;
    
private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    fw::Image image;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    uint32_t framebufferSize = 0;
};
