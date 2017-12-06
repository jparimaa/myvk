#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace fw {

class SwapChain {
public:
    SwapChain();
    ~SwapChain();
    SwapChain(const SwapChain&) = delete;
    SwapChain(SwapChain&&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    SwapChain& operator=(SwapChain&&) = delete;

    bool initialize(uint32_t width, uint32_t height);

private:
    bool createSwapChain(uint32_t width, uint32_t height);

    VkExtent2D swapChainExtent;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
};

} // namespace fw
