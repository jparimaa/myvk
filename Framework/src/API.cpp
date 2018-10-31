#include "API.h"
#include "Common.h"

namespace fw
{
Framework* API::s_framework = nullptr;

bool API::initializeSwapChain()
{
    return s_framework->m_swapChain.initialize();
}

bool API::initializeSwapChainWithDefaultFramebuffer(VkRenderPass renderPass)
{
    return s_framework->m_swapChain.initializeWithDefaultFramebuffer(renderPass);
}

bool API::initializeGUI(VkDescriptorPool descriptorPool)
{
    return s_framework->m_gui.initialize(descriptorPool);
}

VkFormat API::getSwapChainImageFormat()
{
    return s_framework->m_swapChain.getImageFormat();
}

VkExtent2D API::getSwapChainExtent()
{
    return s_framework->m_swapChain.getExtent();
}

uint32_t API::getSwapChainImageCount()
{
    return s_framework->m_swapChain.getImageCount();
}

uint32_t API::getCurrentSwapChainImageIndex()
{
    return s_framework->m_currentImageIndex;
}

const std::vector<VkFramebuffer>& API::getSwapChainFramebuffers()
{
    return s_framework->m_swapChain.getFramebuffers();
}

const std::vector<VkImageView>& API::getSwapChainImageViews()
{
    return s_framework->m_swapChain.getImageViews();
}

const VkImageView& API::getSwapChainDepthImageView()
{
    return s_framework->m_swapChain.getDepthImageView();
}

VkCommandPool API::getCommandPool()
{
    return s_framework->m_commandPool;
}

float API::getTimeSinceStart()
{
    return s_framework->m_time.getSinceStart();
}

float API::getTimeDelta()
{
    return s_framework->m_time.getDelta();
}

bool API::isKeyPressed(int key)
{
    return s_framework->m_input.isKeyPressed(key);
}

bool API::isKeyDown(int key)
{
    return s_framework->m_input.isKeyDown(key);
}

bool API::isKeyReleased(int key)
{
    return s_framework->m_input.isKeyReleased(key);
}

float API::getMouseDeltaX()
{
    return s_framework->m_input.getDeltaX();
}

float API::getMouseDeltaY()
{
    return s_framework->m_input.getDeltaY();
}

void API::setCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers)
{
    if (commandBuffers.size() != getSwapChainImageCount())
    {
        printError("Setting all command buffers but the size does not match swap chain image count");
    }
    s_framework->m_commandBuffers = commandBuffers;
}

void API::setNextCommandBuffer(VkCommandBuffer commandBuffer)
{
    s_framework->m_nextCommandBuffer = commandBuffer;
}

void API::setRenderBufferFence(VkFence fence)
{
    s_framework->m_renderBufferFence = fence;
}

GLFWwindow* API::getGLFWwindow()
{
    return s_framework->m_window.getWindow();
}

} // namespace fw
