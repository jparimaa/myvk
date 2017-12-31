#include "Command.h"
#include "Context.h"
#include "Common.h"

namespace fw
{

bool Command::createDefaultCommandPool(VkCommandPool* commandPool)
{
    QueueFamilyIndices indices = getQueueFamilies(Context::getPhysicalDevice(), Context::getSurface());

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = 0;  // Optional

    if (VkResult r = vkCreateCommandPool(Context::getLogicalDevice(), &poolInfo, nullptr, commandPool);
        r != VK_SUCCESS) {
        printError("Failed to create command pool", &r);
        return false;
    }
    return true;
}

} // namespace fw


