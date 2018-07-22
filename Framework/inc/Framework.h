#pragma once

#include "Instance.h"
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Time.h"
#include "Input.h"
#include "GUI.h"
#include "Application.h"

#include <vulkan/vulkan.h>

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
    void setApplication(Application* application);
    void execute();

private:
    Instance m_instance;
    Window m_window;
    Device m_device;
    SwapChain m_swapChain;
    Time m_time;
    Input m_input;
    GUI m_gui;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkSemaphore m_imageAvailable = VK_NULL_HANDLE;
    VkSemaphore m_renderFinished = VK_NULL_HANDLE;

    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChainHandle = VK_NULL_HANDLE;

    Application* m_app = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;

    bool createSemaphores();
    bool render();
};

} // namespace fw
