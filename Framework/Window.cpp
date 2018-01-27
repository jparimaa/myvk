#include "Window.h"
#include "Common.h"
#include "Context.h"

namespace fw
{

Window::~Window()
{
    vkDestroySurfaceKHR(Context::getInstance(), surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::initialize()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    glfwSetWindowPos(window, 1200, 200);

    if (VkResult r = glfwCreateWindowSurface(Context::getInstance(), window, nullptr, &surface);
        r != VK_SUCCESS) {
        printError("Failed to create window surface", &r);
        return false;
    }

    Context::surface = surface;
    return true;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

int Window::getWidth() const
{
    return width;
}

int Window::getHeight() const
{
    return height;
}

GLFWwindow* Window::getWindow()
{
    return window;
}

} // namespace fw
