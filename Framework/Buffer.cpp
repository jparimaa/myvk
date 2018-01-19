#include "Buffer.h"
#include "Command.h"

namespace fw
{

void Buffer::copy(Buffer& src, Buffer& dst, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &copyRegion);

    Command::endSingleTimeCommands(commandBuffer);
}

Buffer::~Buffer()
{
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(logicalDevice, buffer, nullptr);
    } else {
        printWarning("Trying to destroy a null buffer");
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(logicalDevice, memory, nullptr);
    } else {
        printWarning("Trying to destroy a null memory");
    }
}

bool Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    logicalDevice = Context::getLogicalDevice();
        
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (VkResult r = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);
        r != VK_SUCCESS) {
        printError("Failed to create buffer", &r);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!findMemoryType(memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex)) {
        return false;
    }

    if (VkResult r = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory);
        r != VK_SUCCESS) {
        printError("Failed to allocate buffer memory", &r);
        return false;
    }

    vkBindBufferMemory(logicalDevice, buffer, memory, 0);
    return true;
}

void Buffer::copyToImage(VkImage image, uint32_t width, uint32_t height) const
{
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Command::endSingleTimeCommands(commandBuffer);
}

} // namespace fw


