#include "Device.h"
#include "Common.h"
#include "Context.h"

#include <set>

namespace fw {

namespace {

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

bool hasDeviceExtensionSupport(VkPhysicalDevice physicalDevice) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool isDeviceSuitable(VkPhysicalDevice physicalDevice) {
    VkSurfaceKHR surface = Context::getSurface();
    QueueFamilyIndices indices = getQueueFamilies(physicalDevice, surface);

    if (hasDeviceExtensionSupport(physicalDevice)) {
        SwapChainSupport swapChainSupport = getSwapChainSupport(physicalDevice, surface);
        bool isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
        return indices.hasGraphicsAndPresentFamily() && isSwapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    return false;
}

}  // namespace

Device::Device() {}

Device::~Device() {}

bool Device::initialize() {
    return getPhysicalDevice();
}

bool Device::getPhysicalDevice() {
    uint32_t deviceCount = 0;
    VkInstance instance = Context::getInstance();
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        printError("Failed to find a GPU with Vulkan support");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            return true;
        }
    }

    printError("Did not find a suitable GPU");
    return false;
}

}  // namespace fw
