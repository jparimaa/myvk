#ifndef TESTAPP_H
#define TESTAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;
    };

    ~TestApp();
    void run();

   private:
    int windowWidth = 800;
    int windowHeight = 600;
    GLFWwindow* window = nullptr;

    VkInstance instance = nullptr;
    VkDebugReportCallbackEXT callback = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = nullptr;

    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue = nullptr;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

    void init();
    void drawFrame();
    void createWindow();
    void createInstance();
    void createDebugReportCallback();
    void createSurface();
    void getPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void recreateSwapChain();
    void clearSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createCommandBuffers();
    void createSemaphores();

    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device) const;
    bool hasDeviceExtensionSupport(VkPhysicalDevice device) const;
    SwapChainSupportDetails getSwapChainSupport(VkPhysicalDevice device) const;
    VkShaderModule createShaderModule(const std::vector<char>& code) const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

#endif