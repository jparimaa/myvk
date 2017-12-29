#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>
#include <vector>

namespace fw
{

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

void printError(std::string_view msg, const VkResult* result = nullptr);

struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentFamily = -1;
    bool hasGraphicsAndPresentFamily() const;
};

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

struct SwapChainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupport getSwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkShaderModule createShaderModule(const std::string& filename);
    
} // namespace fw
