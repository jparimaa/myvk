#include "Instance.h"
#include "Common.h"
#include "Constants.h"
#include "Context.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace fw
{
namespace
{
bool isValidationLayerAvailable()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : Constants::validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> getRequiredExtensions()
{
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; ++i)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    if (Constants::enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT /*flags*/,
                                                    VkDebugReportObjectTypeEXT /*objType*/,
                                                    uint64_t /*obj*/,
                                                    size_t /*location*/,
                                                    int32_t /*code*/,
                                                    const char* /*layerPrefix*/,
                                                    const char* msg,
                                                    void* /*userData*/)
{
    std::cerr << "Validation layer: " << msg << "\n";
    return VK_FALSE;
}

} // unnamed

Instance::~Instance()
{
    auto destroyDebugReportCallback
        = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
    if (destroyDebugReportCallback != nullptr)
    {
        destroyDebugReportCallback(m_instance, m_callback, nullptr);
    }
    vkDestroyInstance(m_instance, nullptr);
}

bool Instance::initialize()
{
    return createInstance() && createDebugReportCallback();
}

bool Instance::createInstance()
{
    if (Constants::enableValidationLayers && !isValidationLayerAvailable())
    {
        printError("Validation layers requested but not available");
        return false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MyApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = fw::ui32size(extensions);
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (Constants::enableValidationLayers)
    {
        createInfo.enabledLayerCount = fw::ui32size(Constants::validationLayers);
        createInfo.ppEnabledLayerNames = Constants::validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (VkResult r = vkCreateInstance(&createInfo, nullptr, &m_instance); r != VK_SUCCESS)
    {
        printError("Failed to create instance", &r);
        return false;
    }

    Context::s_instance = m_instance;
    return true;
}

bool Instance::createDebugReportCallback()
{
    if (!Constants::enableValidationLayers)
    {
        return true;
    }

    VkDebugReportCallbackCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags
        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    auto createDebugReportCallback
        = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
    if (createDebugReportCallback != nullptr)
    {
        createDebugReportCallback(m_instance, &createInfo, nullptr, &m_callback);
    }
    else
    {
        printError("Failed to set up debug callback");
        return false;
    }
    return true;
}

} // namespace fw
