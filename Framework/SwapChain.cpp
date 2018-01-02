#include "SwapChain.h"
#include "Common.h"
#include "Context.h"
#include "Constants.h"
#include "Image.h"

#include <set>
#include <iostream>
#include <algorithm>
#include <array>

namespace fw
{

namespace
{

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
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
    if (formatCount != 0) {
        capabilities.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, capabilities.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        capabilities.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, capabilities.presentModes.data());
    }

    return capabilities;
}

SwapChain::~SwapChain()
{
    VkDevice logicalDevice = Context::getLogicalDevice();
    
    vkDestroyImage(logicalDevice, depthImage, nullptr);
    vkDestroyImageView(logicalDevice, depthImageView, nullptr);
    vkFreeMemory(logicalDevice, depthImageMemory, nullptr);
    
    for (size_t i = 0; i < imageViews.size(); ++i) {        
        vkDestroyImageView(logicalDevice, imageViews[i], nullptr);
        vkDestroyFramebuffer(logicalDevice, framebuffers[i], nullptr);
    }
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr); // Destroys images
}

bool SwapChain::create(uint32_t width, uint32_t height)
{
    Capabilities capabilities = getCapabilities(Context::getPhysicalDevice(), Context::getSurface());
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(capabilities.formats);
    imageFormat = surfaceFormat.format;
    VkPresentModeKHR presentMode = chooseSwapPresentMode(capabilities.presentModes);
    extent = chooseSwapExtent(capabilities.surfaceCapabilities, width, height);

    imageCount = capabilities.surfaceCapabilities.minImageCount + 1;
    uint32_t maxImageCount = capabilities.surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount) {
        imageCount = maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Context::getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = getQueueFamilies(Context::getPhysicalDevice(), Context::getSurface());
    uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (VkResult r = vkCreateSwapchainKHR(Context::getLogicalDevice(), &createInfo, nullptr, &swapChain);
        r != VK_SUCCESS) {
        printError("Failed to create swap chain", &r);
        return false;
    }
    
    return true;
}

bool SwapChain::initialize(VkRenderPass renderPass)
{
    return
        createImageViews() &&
        createDepthImage() &&
        createFramebuffers(renderPass);
}

VkFormat SwapChain::getImageFormat() const
{
    return imageFormat;
}

VkExtent2D SwapChain::getExtent() const
{
    return extent;
}

bool SwapChain::createImageViews()
{
    vkGetSwapchainImagesKHR(Context::getLogicalDevice(), swapChain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(Context::getLogicalDevice(), swapChain, &imageCount, images.data());

    imageViews.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (VkResult r = vkCreateImageView(Context::getLogicalDevice(), &createInfo, nullptr, &imageViews[i]);
            r != VK_SUCCESS) {
            printError("ERROR: Failed to create an image view", &r);
            return false;
        }
    }    
    return true;
}

bool SwapChain::createDepthImage()
{
    VkFormat format = Constants::depthFormat;
    bool success = true;
    success = success && Image::createImage(extent.width, extent.height, format,
                                            VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                            &depthImage, &depthImageMemory);
    success = success && Image::createImageView(depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);
    success = success && Image::transitImageLayout(depthImage,
                                                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    return success;
}

bool SwapChain::createFramebuffers(VkRenderPass renderPass)
{
    framebuffers.resize(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); ++i) {
        std::array<VkImageView, 2> attachments = {
            imageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (VkResult r = vkCreateFramebuffer(Context::getLogicalDevice(), &framebufferInfo, nullptr, &framebuffers[i]);
            r != VK_SUCCESS) {
            printError("Failed to create framebuffer", &r);
            return false;
        }
    }
    return true;
}
    
}  // namespace fw
