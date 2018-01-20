#include "API.h"

namespace fw
{

Framework* API::framework = nullptr;

bool API::initializeSwapChain(VkRenderPass renderPass)
{
    return framework->initializeSwapChain(renderPass);
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

void API::setCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers)
{
    framework->commandBuffers = commandBuffers;
}

} // namespace fw


