#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>
#include <vector>

namespace fw {

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;
    bool hasGraphicsAndPresentFamily() const;
};

struct SwapChainSupport {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

SwapChainSupport getSwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

void printError(std::string_view msg, const VkResult* result = nullptr);
    
} // namespace fw
