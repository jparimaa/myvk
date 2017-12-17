#include "API.h"

namespace fw {

Framework* API::framework = nullptr;

VkFormat API::getSwapChainImageFormat() {
    return framework->swapChain.getImageFormat();
}

VkExtent2D API::getSwapChainExtent() {
    return framework->swapChain.getExtent();
}

} // namespace fw


