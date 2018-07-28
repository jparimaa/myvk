#include "Input.h"

namespace
{
bool isKeyTrueInMap(const std::unordered_map<int, bool>& container, int key)
{
    auto it = container.find(key);
    return it != container.end() && it->second;
}

} // unnamed

namespace fw
{
bool Input::initialize(GLFWwindow* window)
{
    if (!window)
    {
        return false;
    }
    m_window = window;

    auto keyCallback = [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        static_cast<Input*>(glfwGetWindowUserPointer(win))->handleKey(win, key, scancode, action, mods);
    };

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwGetCursorPos(window, &m_x, &m_y);
    return true;
}

bool Input::isKeyPressed(int key) const
{
    return isKeyTrueInMap(m_pressed, key);
}

bool Input::isKeyDown(int key) const
{
    return isKeyTrueInMap(m_down, key);
}

bool Input::isKeyReleased(int key) const
{
    return isKeyTrueInMap(m_released, key);
}

float Input::getDeltaX() const
{
    return static_cast<float>(m_deltaX);
}

float Input::getDeltaY() const
{
    return static_cast<float>(m_deltaY);
}

void Input::update()
{
    double newX, newY;
    glfwGetCursorPos(m_window, &newX, &newY);
    m_deltaX = m_x - newX;
    m_deltaY = m_y - newY;
    m_x = newX;
    m_y = newY;
}

void Input::clearKeyStatus()
{
    m_pressed.clear();
    m_released.clear();
}

void Input::handleKey(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action == GLFW_PRESS)
    {
        m_pressed[key] = true;
        m_down[key] = true;
    }
    if (action == GLFW_RELEASE)
    {
        m_released[key] = true;
        m_down[key] = false;
    }
    if (action == GLFW_REPEAT)
    {
        m_down[key] = true;
    }
}

} // namespace fw
