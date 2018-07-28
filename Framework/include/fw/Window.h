#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw
{
class Window
{
public:
    Window(){};
    ~Window();
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    bool initialize();
    bool shouldClose() const;
    void pollEvents() const;

    int getWidth() const;
    int getHeight() const;

    GLFWwindow* getWindow();

private:
    int m_width = 1600;
    int m_height = 1200;
    GLFWwindow* m_window = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};

} // namespace fw
