#include "Framework.h"

#include <iostream>

namespace fw {

Framework::Framework() {}

bool Framework::initialize() {
    glfwInit();
    bool success = true;
    success = success && instance.initialize();
    success = success && window.initialize();
    success = success && device.initialize();
    return success;
}

void Framework::execute() {
    while (!window.shouldClose()) {
        window.pollEvents();
    }
}

} // namespace fw
