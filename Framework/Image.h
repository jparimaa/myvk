#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Image
{
public:
    Image() = delete;
    bool createSimpleImageToMemory(uint32_t width, uint32_t height, VkFormat format,
                                   VkImageTiling tiling, VkImageUsageFlags usage,
                                   VkImage* image, VkDeviceMemory* imageMemory);
    bool createSimpleImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView);
    bool transitImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
};

} // namespace fw
