#include "Framework.h"

#include <iostream>

namespace fw {

Framework::Framework() {}

bool Framework::initialize() {
    bool success = true;
    success = success && context.initialize();
    success = success && window.initialize(context.getInstance());
    return success;
}

void Framework::execute() {
    while (!window.shouldClose()) {
        window.pollEvents();
    }
}

} // namespace fw
