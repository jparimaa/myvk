#include "Time.h"

#include <GLFW/glfw3.h>

namespace fw
{

void Time::update()
{
    float t = static_cast<float>(glfwGetTime());
    m_delta = t - m_sinceStart;
    m_sinceStart = t;
}

float Time::getSinceStart() const
{
    return m_sinceStart;
}

float Time::getDelta() const
{
    return m_delta;
}

}  // namespace fw
