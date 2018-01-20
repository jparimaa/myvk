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
    Texture(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = delete;
    
    bool load(const std::string& filename);

    VkImageView getImageView() const;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    Image image;    
    VkImageView imageView = VK_NULL_HANDLE;
};

} // namespace fw
