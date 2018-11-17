#include "Image.h"
#include "Command.h"
#include "Common.h"
#include "Context.h"

namespace fw
{
Image::~Image()
{
    if (m_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_logicalDevice, m_image, nullptr);
    }

    if (m_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_logicalDevice, m_memory, nullptr);
    }
}

bool Image::create(const VkImageCreateInfo& imageInfo)
{
    m_logicalDevice = Context::getLogicalDevice();
    return allocate(imageInfo);
}

bool Image::create(uint32_t width, uint32_t height, VkFormat format, VkImageCreateFlags flags, VkImageUsageFlags usage)
{
    return create(width, height, format, flags, usage, 1, 1);
}

bool Image::create(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageCreateFlags flags,
                   VkImageUsageFlags usage,
                   uint32_t arrayLayers)
{
    return create(width, height, format, flags, usage, arrayLayers, 1);
}

bool Image::create(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageCreateFlags flags,
                   VkImageUsageFlags usage,
                   uint32_t arrayLayers,
                   uint32_t mipLevels)
{
    m_logicalDevice = Context::getLogicalDevice();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = flags;

    return allocate(imageInfo);
}

bool Image::createView(VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (VkResult r = vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, imageView); r != VK_SUCCESS)
    {
        printError("Failed to create image view", &r);
        return false;
    }
    return true;
}

bool Image::transitLayout(VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_layout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (m_layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (m_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (m_layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask
            = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (m_layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    else
    {
        printError("Unsupported image layout transition");
        return false;
    }

    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    Command::endSingleTimeCommands(commandBuffer);

    m_layout = newLayout;

    return true;
}

VkImage Image::getHandle() const
{
    return m_image;
}

bool Image::allocate(const VkImageCreateInfo& imageInfo)
{
    if (VkResult r = vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &m_image); r != VK_SUCCESS)
    {
        printError("Failed to create image", &r);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_logicalDevice, m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocInfo.memoryTypeIndex))
    {
        return false;
    }

    if (VkResult r = vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &m_memory); r != VK_SUCCESS)
    {
        printError("Failed to allocate image memory", &r);
    }

    vkBindImageMemory(m_logicalDevice, m_image, m_memory, 0);
    return true;
}

} // namespace fw
