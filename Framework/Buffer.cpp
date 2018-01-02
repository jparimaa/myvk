#include "Buffer.h"
#include "Common.h"
#include "Command.h"
#include "Context.h"

namespace fw
{

bool Buffer::create(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer* buffer,
                    VkDeviceMemory* bufferMemory)
{
    VkDevice logicalDevice = Context::getLogicalDevice();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (VkResult r = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, buffer);
        r != VK_SUCCESS) {
        printError("Failed to create buffer", &r);
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!findMemoryType(memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex)) {
        return false;
    }

    if (VkResult r = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, bufferMemory);
        r != VK_SUCCESS) {
        printError("Failed to allocate buffer memory", &r);
        return false;
    }

    vkBindBufferMemory(logicalDevice, *buffer, *bufferMemory, 0);
    return true;
}

void Buffer::copy(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    Command::endSingleTimeCommands(commandBuffer);
}

} // namespace fw


