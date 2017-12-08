#pragma once

#include "Window.h"
#include "Instance.h"
#include "Device.h"
#include "SwapChain.h"
#include "Application.h"

namespace fw {

class Framework {
public:
    Framework();
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

    Application* app = nullptr;
};

} // namespace fw
