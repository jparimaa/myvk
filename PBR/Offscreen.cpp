#include "Offscreen.h"

#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/Macros.h"
#include "../Framework/Command.h"

VkRenderPass Offscreen::renderPass = VK_NULL_HANDLE;
VkFormat Offscreen::format = VK_FORMAT_UNDEFINED;

Offscreen::~Offscreen()
{
    vkDestroyImageView(logicalDevice, imageView, nullptr);
    vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
}

bool Offscreen::createFramebuffer(uint32_t size, uint32_t layerCount, uint32_t levelCount)
{
    logicalDevice = fw::Context::getLogicalDevice();
    
    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.create(size, size, format, 0, usage, 1);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.flags = 0;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;
    viewInfo.image = image.getHandle();
    if (VkResult r = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);
        r != VK_SUCCESS) {        
        fw::printError("Failed to create image view for HDR equirectangular offscreen", &r);
        return false;
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView;
    framebufferInfo.width = size;
    framebufferInfo.height = size;
    framebufferInfo.layers = 1;
    if (VkResult r = vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffer);
        r != VK_SUCCESS) {
        fw::printError("Failed to create an offscreen framebuffer", &r);
        return false;
    }

    VkCommandBuffer commandBuffer = fw::Command::beginSingleTimeCommands();
    
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.image = image.getHandle();
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    fw::Command::endSingleTimeCommands(commandBuffer);
    framebufferSize = size;

    return true;
}

uint32_t Offscreen::getSize() const
{
    return framebufferSize;
}

VkImage Offscreen::getImageHandle() const
{
    return image.getHandle();
}

VkFramebuffer Offscreen::getFramebuffer() const
{
    return framebuffer;
}
