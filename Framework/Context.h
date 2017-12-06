#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Context {
public:
    friend class Instance;
    friend class Window;
    friend class Device;
    
    Context() = delete;
    static VkInstance getInstance();
    static VkSurfaceKHR getSurface();
    static VkPhysicalDevice getPhysicalDevice();
    static VkDevice getLogicalDevice();

private:
    static VkInstance instance;
    static VkSurfaceKHR surface;
    static VkPhysicalDevice physicalDevice;
    static VkDevice logicalDevice;
};

} // namespace fw
