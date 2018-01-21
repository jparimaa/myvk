#include "Time.h"

#include <GLFW/glfw3.h>

namespace fw
{

void Time::update()
{
    float t = static_cast<float>(glfwGetTime());
    delta = t - sinceStart;
    sinceStart = t;
}

float Time::getSinceStart() const
{
    return sinceStart;
}

float Time::getDelta() const
{
    return delta;
}

}  // namespace fw
