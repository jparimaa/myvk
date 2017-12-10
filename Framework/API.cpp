#include "API.h"

namespace fw {

Framework* API::framework = nullptr;

VkFormat API::getSwapChainImageFormat() {
    return framework->swapChain.getImageFormat();
}

} // namespace fw


