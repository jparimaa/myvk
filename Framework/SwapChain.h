#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace fw {

class SwapChain {
public:
    SwapChain();
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    bool initialize(uint32_t width, uint32_t height);

    VkFormat getImageFormat() const;

private:
    VkExtent2D extent;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkFormat imageFormat;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    bool createSwapChain(uint32_t width, uint32_t height);
    bool createImageViews();
};

} // namespace fw
