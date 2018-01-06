#include "Texture.h"
#include "Common.h"
#include "Buffer.h"
#include "Context.h"
#include "Image.h"
#include "Command.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include <stb_image.h>
#pragma GCC diagnostic pop

namespace fw
{

namespace
{

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = Command::beginSingleTimeCommands();

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

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Command::endSingleTimeCommands(commandBuffer);
}

} // unnamed

Texture::Texture() :
    logicalDevice(Context::getLogicalDevice())
{
}

Texture::~Texture()
{
    vkDestroyImageView(logicalDevice, textureImageView, nullptr);
    vkDestroyImage(logicalDevice, textureImage, nullptr);
    vkFreeMemory(logicalDevice, textureImageMemory, nullptr);
}

bool Texture::load(const std::string& filename)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        printError("Failed to load texture image");
        return false;
    }

    Cleaner cleaner([&pixels]() { stbi_image_free(pixels); });
    
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    Buffer::Staging staging;
    if (!Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        staging)) {
        return false;
    }
    
    void* data;
    if (VkResult r = vkMapMemory(logicalDevice, staging.memory, 0, imageSize, 0, &data);
        r != VK_SUCCESS) {
        printError("Failed to map memory for image");
        return false;
    }
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logicalDevice, staging.memory);

    if (!Image::create(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
                       VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       &textureImage, &textureImageMemory)) {
        return false;
    }

    if (!Image::transitLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
        return false;
    }
    
    copyBufferToImage(staging.buffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    if (!Image::transitLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
        return false;
    }
    
    if (!Image::createView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &textureImageView)) {
        return false;
    }
    
    return true;
}

} // namespace fw


