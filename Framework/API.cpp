#include "API.h"

namespace fw
{

Framework* API::framework = nullptr;

bool API::initializeSwapChain(VkRenderPass renderPass)
{
    return framework->swapChain.initialize(renderPass);
}

VkFormat API::getSwapChainImageFormat()
{
    return framework->swapChain.getImageFormat();
}

VkExtent2D API::getSwapChainExtent()
{
    return framework->swapChain.getExtent();
}

const std::vector<VkFramebuffer>& API::getSwapChainFramebuffers()
{
    return framework->swapChain.getFramebuffers();
}

VkCommandPool API::getCommandPool()
{
    return framework->commandPool;
}

float API::getTimeSinceStart()
{
    return framework->time.getSinceStart();
}

float API::getTimeDelta()
{
    return framework->time.getDelta();
}

bool API::isKeyPressed(int key)
{
    return framework->input.isKeyPressed(key);
}

bool API::isKeyDown(int key)
{
    return framework->input.isKeyDown(key);
}

bool API::isKeyReleased(int key)
{
    return framework->input.isKeyReleased(key);
}

float API::getMouseDeltaX()
{
    return framework->input.getDeltaX();
}

float API::getMouseDeltaY()
{
    return framework->input.getDeltaY();
}

void API::setCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers)
{
    framework->commandBuffers = commandBuffers;
}

GLFWwindow* API::getGLFWwindow()
{
    return framework->window.getWindow();
}

} // namespace fw


