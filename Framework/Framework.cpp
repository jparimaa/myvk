#include "Framework.h"

#include <iostream>

namespace fw {

Framework::Framework() {}

bool Framework::initialize() {
    window.initialize();
    return true;
}

void Framework::execute() {
    while (!window.shouldClose()) {
        window.pollEvents();
    }
}

}  // namespace fw
