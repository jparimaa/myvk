#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Context {
public:
    friend class Instance;
    friend class Window;
    
    Context() = delete;
    static VkInstance getInstance();
    static VkSurfaceKHR getSurface();

private:
    static VkInstance instance;
    static VkSurfaceKHR surface;
};

} // namespace fw
