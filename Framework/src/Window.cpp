#include "Window.h"
#include "Common.h"
#include "Context.h"

namespace fw
{

Window::~Window()
{
    vkDestroySurfaceKHR(Context::getInstance(), m_surface, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::initialize()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);

    glfwSetWindowPos(m_window, 1200, 200);

    if (VkResult r = glfwCreateWindowSurface(Context::getInstance(), m_window, nullptr, &m_surface);
        r != VK_SUCCESS) {
        printError("Failed to create window surface", &r);
        return false;
    }

    Context::s_surface = m_surface;
    return true;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

int Window::getWidth() const
{
    return m_width;
}

int Window::getHeight() const
{
    return m_height;
}

GLFWwindow* Window::getWindow()
{
    return m_window;
}

} // namespace fw
