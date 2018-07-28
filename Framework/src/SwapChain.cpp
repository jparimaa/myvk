#include "SwapChain.h"
#include "Common.h"
#include "Constants.h"
#include "Context.h"
#include "Image.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <set>

namespace fw
{
namespace
{
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {width, height};

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

} // unnamed

SwapChain::Capabilities SwapChain::getCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    Capabilities capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities.surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        capabilities.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, capabilities.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        capabilities.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice, surface, &presentModeCount, capabilities.presentModes.data());
    }

    return capabilities;
}

SwapChain::~SwapChain()
{
    vkDestroyImageView(m_logicalDevice, m_depthImageView, nullptr);

    for (VkImageView& iw : m_imageViews)
    {
        vkDestroyImageView(m_logicalDevice, iw, nullptr);
    }
    for (VkFramebuffer& fb : m_framebuffers)
    {
        vkDestroyFramebuffer(m_logicalDevice, fb, nullptr);
    }
    vkDestroySwapchainKHR(m_logicalDevice, m_swapChain,
                          nullptr); // Destroys images
}

bool SwapChain::create(uint32_t width, uint32_t height)
{
    m_logicalDevice = Context::getLogicalDevice();

    Capabilities capabilities = getCapabilities(Context::getPhysicalDevice(), Context::getSurface());
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(capabilities.formats);
    m_imageFormat = surfaceFormat.format;
    VkPresentModeKHR presentMode = chooseSwapPresentMode(capabilities.presentModes);
    m_extent = chooseSwapExtent(capabilities.surfaceCapabilities, width, height);

    m_imageCount = capabilities.surfaceCapabilities.minImageCount + 1;
    uint32_t maxImageCount = capabilities.surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0 && m_imageCount > maxImageCount)
    {
        m_imageCount = maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Context::getSurface();
    createInfo.minImageCount = m_imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = getQueueFamilies(Context::getPhysicalDevice(), Context::getSurface());
    uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (VkResult r = vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain); r != VK_SUCCESS)
    {
        printError("Failed to create swap chain", &r);
        return false;
    }

    return true;
}

bool SwapChain::initialize()
{
    return createImageViews() && createDepthImage();
}

bool SwapChain::initializeWithDefaultFramebuffer(VkRenderPass renderPass)
{
    return createImageViews() && createDepthImage() && createFramebuffers(renderPass);
}

VkFormat SwapChain::getImageFormat() const
{
    return m_imageFormat;
}

VkExtent2D SwapChain::getExtent() const
{
    return m_extent;
}

VkSwapchainKHR SwapChain::getSwapChain() const
{
    return m_swapChain;
}

uint32_t SwapChain::getImageCount() const
{
    return m_imageCount;
}

const std::vector<VkFramebuffer>& SwapChain::getFramebuffers() const
{
    return m_framebuffers;
}

const std::vector<VkImageView>& SwapChain::getImageViews() const
{
    return m_imageViews;
}

const VkImageView& SwapChain::getDepthImageView() const
{
    return m_depthImageView;
}

bool SwapChain::createImageViews()
{
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &m_imageCount, nullptr);
    m_images.resize(m_imageCount);
    vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &m_imageCount, m_images.data());

    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (VkResult r = vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_imageViews[i]); r != VK_SUCCESS)
        {
            printError("ERROR: Failed to create an image view", &r);
            return false;
        }
    }
    return true;
}

bool SwapChain::createDepthImage()
{
    VkFormat format = Constants::depthFormat;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    bool success = m_depthImage.create(m_extent.width, m_extent.height, format, 0, imageUsage, 1)
        && m_depthImage.createView(format, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depthImageView)
        && m_depthImage.transitLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    return success;
}

bool SwapChain::createFramebuffers(VkRenderPass renderPass)
{
    m_framebuffers.resize(m_imageViews.size());
    for (size_t i = 0; i < m_imageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {m_imageViews[i], m_depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = fw::ui32size(attachments);
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        if (VkResult r = vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffers[i]);
            r != VK_SUCCESS)
        {
            printError("Failed to create a swapchain framebuffer", &r);
            return false;
        }
    }
    return true;
}

} // namespace fw
