#pragma once

#include <GLFW/glfw3.h>

#include <unordered_map>

namespace fw
{
class Input
{
public:
    Input(){};
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&) = delete;

    bool initialize(GLFWwindow* window);

    bool isKeyPressed(int key) const;
    bool isKeyDown(int key) const;
    bool isKeyReleased(int key) const;
    float getDeltaX() const;
    float getDeltaY() const;

    void update();
    void clearKeyStatus();

private:
    GLFWwindow* m_window = nullptr;
    double m_x = 0.0;
    double m_y = 0.0;
    double m_deltaX = 0.0;
    double m_deltaY = 0.0;
    std::unordered_map<int, bool> m_pressed;
    std::unordered_map<int, bool> m_down;
    std::unordered_map<int, bool> m_released;

    void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace fw
