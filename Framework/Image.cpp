#include "Image.h"
#include "Context.h"
#include "Common.h"

namespace fw
{

bool Image::createSimpleImageToMemory(uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkImageTiling tiling,
                                      VkImageUsageFlags usage,
                                      VkImage* image,
                                      VkDeviceMemory* imageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkDevice logicalDevice = Context::getLogicalDevice();
    if (VkResult r = vkCreateImage(logicalDevice, &imageInfo, nullptr, image);
        r != VK_SUCCESS) {
        printError("Failed to create image", &r);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocInfo.memoryTypeIndex)) {
        return false;
    }

    if (VkResult r = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, imageMemory);
        r != VK_SUCCESS) {
        printError("Failed to allocate image memory");
    }

    vkBindImageMemory(logicalDevice, *image, *imageMemory, 0);
    return true;
}

} // namespace fw


