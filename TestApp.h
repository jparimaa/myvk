#ifndef TESTAPP_H
#define TESTAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class TestApp {
   public:
    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool isComplete() const;
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

    void init();
    void createWindow();
    void createInstance();
    void createDebugReportCallback();
    void createSurface();
    void getPhysicalDevice();
    void createLogicalDevice();
    TestApp::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};

#endif