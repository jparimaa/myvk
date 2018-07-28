#pragma once

#include "Image.h"

#include <vulkan/vulkan.h>

#include <string>

namespace fw
{
class Texture
{
public:
    Texture(){};
    ~Texture();

    bool load(const std::string& filename, VkFormat format);
    bool loadHDR(const std::string& filename);
    bool load(const unsigned char* data, size_t size, VkFormat format);

    VkImageView getImageView() const;

private:
    Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;

    bool load(const std::string& filename, VkFormat format, int desiredChannels);
    bool createImage(unsigned char* pixels, int width, int height, VkFormat format);
};

} // namespace fw
