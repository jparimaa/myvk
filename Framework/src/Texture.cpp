#include "Texture.h"
#include "Buffer.h"
#include "Common.h"
#include "Context.h"

#define STB_IMAGE_IMPLEMENTATION
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <stb_image.h>
#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include <cstring>

namespace fw
{
Texture::~Texture()
{
    if (m_imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(Context::getLogicalDevice(), m_imageView, nullptr);
    }
}

bool Texture::load(const std::string& filename, VkFormat format)
{
    return load(filename, format, STBI_rgb_alpha);
}

bool Texture::loadHDR(const std::string& filename)
{
    return load(filename, VK_FORMAT_R16G16B16A16_SFLOAT, 0);
}

bool Texture::load(const unsigned char* data, size_t size, VkFormat format)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels
        = stbi_load_from_memory(data, static_cast<int>(size), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
    {
        printError("Failed to load texture from data");
        return false;
    }

    Cleaner cleaner([&pixels]() { stbi_image_free(pixels); });

    return createImage(pixels, texWidth, texHeight, format);
}

VkImageView Texture::getImageView() const
{
    return m_imageView;
}

bool Texture::load(const std::string& filename, VkFormat format, int desiredChannels)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, desiredChannels);
    if (!pixels)
    {
        printError("Failed to load texture image: " + filename);
        return false;
    }

    Cleaner cleaner([&pixels]() { stbi_image_free(pixels); });

    return createImage(pixels, texWidth, texHeight, format);
}

bool Texture::createImage(unsigned char* pixels, int width, int height, VkFormat format)
{
    VkDeviceSize imageSize = width * height * 4;
    Buffer staging;
    VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (!staging.create(imageSize, bufferUsage, properties))
    {
        return false;
    }

    if (!staging.setData<stbi_uc>(imageSize, pixels))
    {
        return false;
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (!m_image.create(width, height, format, 0, imageUsage, 1))
    {
        return false;
    }

    if (!m_image.transitLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
    {
        return false;
    }

    staging.copyToImage(m_image.getHandle(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    if (!m_image.transitLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
    {
        return false;
    }

    if (!m_image.createView(format, VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView))
    {
        return false;
    }
    return true;
}

} // namespace fw
