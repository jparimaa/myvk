#pragma once

#include "Window.h"
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "Application.h"

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
    VkCommandPool commandPool = VK_NULL_HANDLE;

    Application* app = nullptr;

    bool initializeSwapChain(VkRenderPass renderPass);
};

} // namespace fw
