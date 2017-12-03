#include "Window.h"
#include "Common.h"
#include "Context.h"

namespace fw {

Window::Window() {}

Window::~Window() {
    vkDestroySurfaceKHR(Context::getInstance(), surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::initialize() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    if (VkResult r = glfwCreateWindowSurface(Context::getInstance(), window, nullptr, &surface);
        r != VK_SUCCESS) {
        printError("Failed to create window surface", &r);
        return false;
    }

    Context::surface = surface;
    return true;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

} // namespace fw
