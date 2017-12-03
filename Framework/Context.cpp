#include "Context.h"
#include "Common.h"

#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace fw {

namespace {

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

bool isValidationLayerAvailable() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> getRequiredExtensions() {
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; ++i) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
};


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                                    VkDebugReportObjectTypeEXT objType,
                                                    uint64_t obj,
                                                    size_t location,
                                                    int32_t code,
                                                    const char* layerPrefix,
                                                    const char* msg,
                                                    void* userData) {
    std::cerr << "Validation layer: " << msg << "\n";
    return VK_FALSE;
}

} // unnamed

Context::Context() {}

Context::~Context() {
    auto destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (destroyDebugReportCallback != nullptr) {
        destroyDebugReportCallback(instance, callback, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
}

bool Context::initialize() {
    return createInstance() && createDebugReportCallback();
}

const VkInstance* Context::getInstance() const {
    return &instance;    
}

bool Context::createInstance() {
    if (enableValidationLayers && !isValidationLayerAvailable()) {
        printError("Validation layers requested but not available");
        return false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MyApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (VkResult r = vkCreateInstance(&createInfo, nullptr, &instance);
        r != VK_SUCCESS) {
        printError("Failed to create instance", &r);
        return false;
    }
    
    return true;
}

bool Context::createDebugReportCallback() {
    if (!enableValidationLayers) {
        return true;
    }
    
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    auto createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (createDebugReportCallback != nullptr) {
        createDebugReportCallback(instance, &createInfo, nullptr, &callback);
    } else {
        printError("Failed to set up debug callback");
        return false;
    }
    return true;
}

} // namespace fw
