#pragma once

#include "Image.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace fw
{
class SwapChain
{
public:
    struct Capabilities
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static Capabilities getCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    SwapChain(){};
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    bool create(uint32_t width, uint32_t height);
    bool initialize();
    bool initializeWithoutDepthImage();
    bool initializeWithDefaultFramebuffer(VkRenderPass renderPass);

    VkFormat getImageFormat() const;
    VkExtent2D getExtent() const;
    VkSwapchainKHR getSwapChain() const;
    uint32_t getImageCount() const;

    const std::vector<VkFramebuffer>& getFramebuffers() const;
    const std::vector<VkImageView>& getImageViews() const;
    const VkImageView& getDepthImageView() const;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    uint32_t m_imageCount = 0;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;

    bool createSwapChain(uint32_t width, uint32_t height);
    bool createImageViews();
    bool createDepthImage();
    bool createFramebuffers(VkRenderPass renderPass);
};

} // namespace fw
