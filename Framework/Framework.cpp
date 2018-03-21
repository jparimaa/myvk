#include "Framework.h"
#include "API.h"
#include "Context.h"
#include "Command.h"
#include "Common.h"

#include <iostream>

namespace fw
{

Framework::Framework()
{
    API::framework = this;
}

Framework::~Framework()
{
    vkDestroySemaphore(logicalDevice, renderFinished, nullptr);
    vkDestroySemaphore(logicalDevice, imageAvailable, nullptr);
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);    
}

bool Framework::initialize()
{
    glfwInit();
    bool success =
        instance.initialize() &&
        window.initialize() &&
        device.initialize() &&
        swapChain.create(window.getWidth(), window.getHeight()) &&
        Command::createGraphicsCommandPool(&commandPool) &&
        createSemaphores() &&
        input.initialize(window.getWindow());
    
    logicalDevice = Context::getLogicalDevice();
    graphicsQueue = Context::getGraphicsQueue();
    presentQueue = Context::getPresentQueue();
    swapChainHandle = swapChain.getSwapChain();
    
    return success;
}

void Framework::setApplication(Application* application)
{
    app = application;
}

void Framework::execute()
{
    while (!window.shouldClose() && !API::isKeyReleased(GLFW_KEY_ESCAPE)) {
        input.clearKeyStatus();
        window.pollEvents();
        input.update();
        time.update();
        app->update();
        if (gui.isInitialized()) {
            gui.beginPass();
            app->onGUI();
        }
        if (!commandBuffers.empty()) {            
            if (!render()) {
                break;
            }
        } else {
            printWarning("Empty command buffer");
        }        
    }
    vkDeviceWaitIdle(logicalDevice);
}

bool Framework::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    auto createSemaphore = [semaphoreInfo](VkSemaphore& semaphore) {
        if (VkResult r = vkCreateSemaphore(Context::getLogicalDevice(), &semaphoreInfo, nullptr, &semaphore);
            r != VK_SUCCESS) {
            printError("Failed to create a semaphore", &r);
            return false;
        }
        return true;
    };

    return createSemaphore(imageAvailable) && createSemaphore(renderFinished);
}

bool Framework::render()
{
    vkQueueWaitIdle(presentQueue);  // Optional sync for validation layers
    
    uint32_t imageIndex;
    uint64_t timeout = std::numeric_limits<uint64_t>::max();
    
    if (VkResult r = vkAcquireNextImageKHR(logicalDevice, swapChainHandle, timeout, imageAvailable, VK_NULL_HANDLE, &imageIndex);
        r != VK_SUCCESS) {
        printError("Failed to acquire swap chain image");
        return false;
    }

    std::vector<VkCommandBuffer> renderCommandBuffers{commandBuffers[imageIndex]};
    if (gui.isInitialized()) {
        gui.render(API::getSwapChainFramebuffers().at(imageIndex));
        renderCommandBuffers.push_back(gui.getCommandBuffer());
    }

    VkSemaphore waitSemaphores[] = {imageAvailable};
    VkSemaphore signalSemaphores[] = {renderFinished};
    
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = ui32size(renderCommandBuffers);
    submitInfo.pCommandBuffers = renderCommandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (VkResult r = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        r != VK_SUCCESS) {
        printError("Failed to submit a draw command buffer", &r);
        return false;
    }

    VkPresentInfoKHR presentInfo = {};
    VkSwapchainKHR swapChains[] = {swapChainHandle};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;  // Optional

    if (VkResult r = vkQueuePresentKHR(presentQueue, &presentInfo);
        r != VK_SUCCESS) {
        printError("Failed to present swap chain image", &r);
        return false;
    }
    
    return true;
}

} // namespace fw
