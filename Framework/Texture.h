#pragma once

#include "Image.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace fw
{

class Texture
{
public:
    Texture() {};
    ~Texture();
    
    bool load(const std::string& filename);
    bool loadHDR(const std::string& filename);

    VkImageView getImageView() const;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    Image image;    
    VkImageView imageView = VK_NULL_HANDLE;

    bool load(const std::string& filename, VkFormat format, int desiredChannels);
};

} // namespace fw
