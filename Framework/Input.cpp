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
    if (!window) {
        return false;
    }
    
    auto keyCallback = [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        static_cast<Input*>(glfwGetWindowUserPointer(w))->handleKey(w, key, scancode, action, mods);
    };
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    return true;
}

bool Input::isKeyPressed(int key) const
{
    return isKeyTrueInMap(pressed, key);
}

bool Input::isKeyDown(int key) const
{
    return isKeyTrueInMap(down, key);
}

bool Input::isKeyReleased(int key) const
{
    return isKeyTrueInMap(released, key);
}

void Input::handleKey(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action == GLFW_PRESS) {
        pressed[key] = true;
        down[key] = true;
    }
    if (action == GLFW_RELEASE) {
        released[key] = true;
        down[key] = false;
    }
    if (action == GLFW_REPEAT) {
        down[key] = true;
    }
}

void Input::clearKeyStatus()
{
    pressed.clear();
    released.clear();
}

} // namespace fw
