#pragma once

#include "Window.h"
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "Time.h"
#include "Input.h"
#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{

class Framework
{
public:
    friend class API;
    
    Framework();
    ~Framework();
    Framework(const Framework&) = delete;
    Framework(Framework&&) = delete;
    Framework& operator=(const Framework&) = delete;
    Framework& operator=(Framework&&) = delete;

    bool initialize();
    void setApplication(Application* app);
    void execute();

private:
    Instance instance;
    Window window;
    Device device;    
    SwapChain swapChain;
    Time time;
    Input input;
    
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapChainHandle = VK_NULL_HANDLE;

    Application* app = nullptr;
    std::vector<VkCommandBuffer> commandBuffers;

    bool initializeSwapChain(VkRenderPass renderPass);
    bool createSemaphores();
    bool render();
};

} // namespace fw
