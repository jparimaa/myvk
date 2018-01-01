#include "Framework.h"
#include "API.h"
#include "Context.h"
#include "Command.h"

#include <iostream>

namespace fw
{

Framework::Framework()
{
    API::framework = this;
}

Framework::~Framework()
{
    vkDestroyCommandPool(Context::getLogicalDevice(), commandPool, nullptr);
}

bool Framework::initialize()
{
    glfwInit();
    bool success = true;
    success = success && instance.initialize();
    success = success && window.initialize();
    success = success && device.initialize();
    success = success && swapChain.create(window.getWidth(), window.getHeight());
    success = success && Command::createGraphicsCommandPool(&commandPool);
    return success;
}

void Framework::setApplication(Application* app)
{
    this->app = app;
}

void Framework::execute()
{
    while (!window.shouldClose()) {
        window.pollEvents();
    }
}

bool Framework::initializeSwapChain(VkRenderPass renderPass)
{
    return swapChain.initialize(renderPass);
}

} // namespace fw
