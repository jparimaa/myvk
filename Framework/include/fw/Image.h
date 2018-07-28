#pragma once

#include <vulkan/vulkan.h>

namespace fw
{
class Image
{
public:
    Image(){};
    ~Image();

    bool create(const VkImageCreateInfo& imageInfo);
    bool create(uint32_t width, uint32_t height, VkFormat format, VkImageCreateFlags flags, VkImageUsageFlags usage);
    bool create(uint32_t width,
                uint32_t height,
                VkFormat format,
                VkImageCreateFlags flags,
                VkImageUsageFlags usage,
                uint32_t arrayLayers);
    bool create(uint32_t width,
                uint32_t height,
                VkFormat format,
                VkImageCreateFlags flags,
                VkImageUsageFlags usage,
                uint32_t arrayLayers,
                uint32_t mipLevels);
    bool createView(VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView);
    bool transitLayout(VkImageLayout newLayout);

    VkImage getHandle() const;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    bool allocate(const VkImageCreateInfo& imageInfo);
};

} // namespace fw
