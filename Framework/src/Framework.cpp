#include "Framework.h"
#include "API.h"
#include "Command.h"
#include "Common.h"
#include "Context.h"

#include <iostream>

namespace fw
{
Framework::Framework()
{
    API::s_framework = this;
}

Framework::~Framework()
{
    vkDestroySemaphore(m_logicalDevice, m_renderFinished, nullptr);
    vkDestroySemaphore(m_logicalDevice, m_imageAvailable, nullptr);
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
}

bool Framework::initialize()
{
    glfwInit();
    bool success = m_instance.initialize() && m_window.initialize() && m_device.initialize()
        && m_swapChain.create(m_window.getWidth(), m_window.getHeight())
        && Command::createGraphicsCommandPool(&m_commandPool) && createSemaphores()
        && m_input.initialize(m_window.getWindow());

    m_logicalDevice = Context::getLogicalDevice();
    m_graphicsQueue = Context::getGraphicsQueue();
    m_presentQueue = Context::getPresentQueue();
    m_swapChainHandle = m_swapChain.getSwapChain();

    return success;
}

void Framework::setApplication(Application* application)
{
    m_app = application;
}

void Framework::execute()
{
    while (!m_window.shouldClose() && !API::isKeyReleased(GLFW_KEY_ESCAPE))
    {
        m_input.clearKeyStatus();
        m_window.pollEvents();
        m_input.update();
        m_time.update();
        if (!acquireNextSwapChainImage())
        {
            break;
        }
        m_app->update();
        if (m_gui.isInitialized())
        {
            m_gui.beginPass();
            m_app->onGUI();
        }
        if (!render())
        {
            break;
        }
    }
    vkDeviceWaitIdle(m_logicalDevice);
}

bool Framework::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    auto createSemaphore = [semaphoreInfo](VkSemaphore& semaphore) {
        if (VkResult r = vkCreateSemaphore(Context::getLogicalDevice(), &semaphoreInfo, nullptr, &semaphore);
            r != VK_SUCCESS)
        {
            printError("Failed to create a semaphore", &r);
            return false;
        }
        return true;
    };

    return createSemaphore(m_imageAvailable) && createSemaphore(m_renderFinished);
}

bool Framework::render()
{
    vkQueueWaitIdle(m_presentQueue); // Optional sync for validation layers

    VkCommandBuffer commandBuffer;
    if (m_nextCommandBuffer != nullptr)
    {
        commandBuffer = m_nextCommandBuffer;
    }
    else if (m_currentImageIndex < m_commandBuffers.size())
    {
        commandBuffer = m_commandBuffers[m_currentImageIndex];
    }
    else
    {
        printError("No command buffer set for rendering");
        return false;
    }

    std::vector<VkCommandBuffer> renderCommandBuffers{commandBuffer};
    if (m_gui.isInitialized())
    {
        m_gui.render(API::getSwapChainFramebuffers().at(m_currentImageIndex));
        renderCommandBuffers.push_back(m_gui.getCommandBuffer());
    }

    VkSemaphore waitSemaphores[] = {m_imageAvailable};
    VkSemaphore signalSemaphores[] = {m_renderFinished};

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = ui32size(renderCommandBuffers);
    submitInfo.pCommandBuffers = renderCommandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (VkResult r = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); r != VK_SUCCESS)
    {
        printError("Failed to submit a draw command buffer", &r);
        return false;
    }

    VkPresentInfoKHR presentInfo{};
    VkSwapchainKHR swapChains[] = {m_swapChainHandle};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_currentImageIndex;
    presentInfo.pResults = nullptr; // Optional

    if (VkResult r = vkQueuePresentKHR(m_presentQueue, &presentInfo); r != VK_SUCCESS)
    {
        printError("Failed to present swap chain image", &r);
        return false;
    }

    return true;
}

bool Framework::acquireNextSwapChainImage()
{
    static uint64_t timeout = std::numeric_limits<uint64_t>::max();

    if (VkResult r = vkAcquireNextImageKHR(m_logicalDevice, m_swapChainHandle, timeout, m_imageAvailable, VK_NULL_HANDLE, &m_currentImageIndex);
        r != VK_SUCCESS)
    {
        printError("Failed to acquire swap chain image");
        return false;
    }
    return true;
}

} // namespace fw
