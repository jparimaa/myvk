#ifndef TESTAPP_H
#define TESTAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class TestApp {
   public:
    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool isComplete() const;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    ~TestApp();
    void run();

   private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    void init();
    void createWindow();
    void createInstance();
    void createDebugReportCallback();
    void createSurface();
    void getPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageView();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();

    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device) const;
    bool hasDeviceExtensionSupport(VkPhysicalDevice device) const;
    SwapChainSupportDetails getSwapChainSupport(VkPhysicalDevice device) const;
    VkShaderModule createShaderModule(const std::vector<char>& code) const;
};

#endif