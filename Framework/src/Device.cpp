#include "Device.h"
#include "Common.h"
#include "Constants.h"
#include "Context.h"
#include "SwapChain.h"

#include <iostream>
#include <set>

namespace fw
{
namespace
{
bool hasDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(Constants::deviceExtensions.begin(), Constants::deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    VkSurfaceKHR surface = Context::getSurface();
    QueueFamilyIndices indices = getQueueFamilies(physicalDevice, surface);

    if (hasDeviceExtensionSupport(physicalDevice))
    {
        SwapChain::Capabilities swapChainCapabilities = SwapChain::getCapabilities(physicalDevice, surface);
        bool isSwapChainAdequate
            = !swapChainCapabilities.formats.empty() && !swapChainCapabilities.presentModes.empty();
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
        return indices.hasGraphicsAndPresentFamily() && isSwapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    return false;
}

} // namespace

Device::~Device()
{
    vkDestroyDevice(logicalDevice, nullptr);
}

bool Device::initialize()
{
    return getPhysicalDevice() && createLogicalDevice();
}

bool Device::getPhysicalDevice()
{
    uint32_t deviceCount = 0;
    VkInstance instance = Context::getInstance();
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        printError("Failed to find a GPU with Vulkan support");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice != VK_NULL_HANDLE)
    {
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        return true;
    }
    printError("Did not find a suitable GPU");
    return false;
}

bool Device::createLogicalDevice()
{
    QueueFamilyIndices indices = getQueueFamilies(physicalDevice, Context::getSurface());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {
        indices.graphicsFamily,
        indices.computeFamily,
        indices.presentFamily};

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = fw::ui32size(queueCreateInfos);
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = fw::ui32size(Constants::deviceExtensions);
    createInfo.ppEnabledExtensionNames = Constants::deviceExtensions.data();
    if (Constants::enableValidationLayers)
    {
        createInfo.enabledLayerCount = fw::ui32size(Constants::validationLayers);
        createInfo.ppEnabledLayerNames = Constants::validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (VkResult r = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice); r != VK_SUCCESS)
    {
        printError("Failed to create a logical device", &r);
        return false;
    }

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.computeFamily, 0, &computeQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);

    Context::s_physicalDevice = physicalDevice;
    Context::s_logicalDevice = logicalDevice;
    Context::s_graphicsQueue = graphicsQueue;
    Context::s_computeQueue = computeQueue;
    Context::s_presentQueue = presentQueue;
    Context::s_physicalDeviceProperties = &physicalDeviceProperties;

    return true;
}

} // namespace fw
