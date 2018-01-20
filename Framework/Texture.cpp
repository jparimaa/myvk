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

#include <cstring>

namespace fw
{

Texture::~Texture()
{
    vkDestroyImageView(logicalDevice, imageView, nullptr);
}

bool Texture::load(const std::string& filename)
{
    logicalDevice = Context::getLogicalDevice();
        
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) {
        printError("Failed to load texture image");
        return false;
    }

    Cleaner cleaner([&pixels]() { stbi_image_free(pixels); });
    
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    Buffer staging;
    VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!staging.create(imageSize, bufferUsage, properties)) {
        return false;
    }

    if (!staging.setData<stbi_uc>(imageSize, pixels)) {
        return false;
    }

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (!image.create(texWidth, texHeight, format, tiling, imageUsage)) {
        return false;
    }

    if (!image.transitLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
        return false;
    }
    
    staging.copyToImage(image.getHandle(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    if (!image.transitLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
        return false;
    }
    
    if (!image.createView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &imageView)) {
        return false;
    }
    
    return true;
}

VkImageView Texture::getImageView() const
{
    return imageView;
}

} // namespace fw


