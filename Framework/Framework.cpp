#include "Framework.h"

#include <iostream>

namespace fw {

Framework::Framework() {}

void Framework::callFramework() const {
    std::cout << "Framework called\n";
}

}  // namespace fw