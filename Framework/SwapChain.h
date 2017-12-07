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
    VkExtent2D swapChainExtent;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkFormat swapChainImageFormat;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    bool createSwapChain(uint32_t width, uint32_t height);
    bool createImageViews();
};

} // namespace fw
