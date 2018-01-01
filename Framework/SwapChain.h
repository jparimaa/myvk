#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace fw
{

class SwapChain
{
public:
    SwapChain() {};
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    bool create(uint32_t width, uint32_t height);
    bool initialize(VkRenderPass renderPass);

    VkFormat getImageFormat() const;
    VkExtent2D getExtent() const;

private:
    VkFormat imageFormat;
    VkExtent2D extent;
    uint32_t imageCount = 0;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    bool createSwapChain(uint32_t width, uint32_t height);
    bool createImageViews();
    bool createDepthImage();
    bool createFramebuffers(VkRenderPass renderPass);
};

} // namespace fw
