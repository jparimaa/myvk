#include "Common.h"
#include "Context.h"

#include <iostream>
#include <fstream>
#include <ios>

namespace fw
{

void printError(std::string_view msg, const VkResult* result)
{
    std::cerr << "ERROR: " << msg << "\n";
    if (result != nullptr) {
        std::cerr << "Result: " << *result << "\n";
    }
}

bool QueueFamilyIndices::hasGraphicsAndPresentFamily() const
{
    return graphicsFamily >= 0 && presentFamily >= 0;
}

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    QueueFamilyIndices indices;
    for (unsigned int i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (queueFamilies[i].queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.hasGraphicsAndPresentFamily()) {
            break;
        }
    }

    return indices;
}

VkShaderModule createShaderModule(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        printError("Failed to open file " + filename);
        return VK_NULL_HANDLE;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
           
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VkShaderModule shaderModule;
    if (VkResult r = vkCreateShaderModule(Context::getLogicalDevice(), &createInfo, nullptr, &shaderModule);
        r != VK_SUCCESS) {
        printError("Failed to create shader module", &r);
        return VK_NULL_HANDLE;
    }
    
    return shaderModule;
}

bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& typeIndex)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(Context::getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            typeIndex = i;
            return true;
        }
    }
    printError("Failed to find a suitable memory type");
    return false;
}

} // namespace fw
