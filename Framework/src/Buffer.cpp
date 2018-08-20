#include "Buffer.h"
#include "Command.h"

namespace fw
{
void Buffer::copy(Buffer& src, Buffer& dst, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src.m_buffer, dst.m_buffer, 1, &copyRegion);

    Command::endSingleTimeCommands(commandBuffer);
}

Buffer::~Buffer()
{
    if (m_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_logicalDevice, m_buffer, nullptr);
    }
    else
    {
        printWarning("Trying to destroy a null buffer");
    }
    if (m_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_logicalDevice, m_memory, nullptr);
    }
    else
    {
        printWarning("Trying to free a null memory");
    }
}

bool Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    m_logicalDevice = Context::getLogicalDevice();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (VkResult r = vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &m_buffer); r != VK_SUCCESS)
    {
        printError("Failed to create buffer", &r);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_logicalDevice, m_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!findMemoryType(memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex))
    {
        return false;
    }

    if (VkResult r = vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &m_memory); r != VK_SUCCESS)
    {
        printError("Failed to allocate buffer memory", &r);
        return false;
    }

    vkBindBufferMemory(m_logicalDevice, m_buffer, m_memory, 0);
    return true;
}

void Buffer::copyToImage(VkImage image, uint32_t width, uint32_t height) const
{
    VkBufferImageCopy region{};
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

    vkCmdCopyBufferToImage(commandBuffer, m_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Command::endSingleTimeCommands(commandBuffer);
}

VkBuffer Buffer::getBuffer() const
{
    return m_buffer;
}

VkDeviceMemory Buffer::getMemory() const
{
    return m_memory;
}

} // namespace fw
