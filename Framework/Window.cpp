#include "Window.h"
#include "Common.h"

namespace fw {

Window::Window() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

Window::~Window() {
    vkDestroySurfaceKHR(*instance, surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::initialize(const VkInstance* instance) {
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    if (glfwCreateWindowSurface(*instance, window, nullptr, &surface) != VK_SUCCESS) {
        printError("Failed to create window surface");
        return false;
    }

    this->instance = instance;
    return true;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

} // namespace fw
