#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Image
{
public:
    Image() {};
    ~Image();
    
    bool create(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
    bool createView(VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView);
    bool transitLayout(VkImageLayout newLayout);

    VkImage getHandle() const;
    
private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

} // namespace fw
