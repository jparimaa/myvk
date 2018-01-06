#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace fw
{

class Texture
{
public:
    Texture();
    ~Texture();
    bool load(const std::string& filename);

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
};

} // namespace fw
