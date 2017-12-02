#include "Framework.h"

#include <iostream>

namespace fw {

Framework::Framework() {}

bool Framework::initialize() {
    window.initialize();
    bool initializationSucceeded = context.initialize();
    return initializationSucceeded;
}

void Framework::execute() {
    while (!window.shouldClose()) {
        window.pollEvents();
    }
}

} // namespace fw
