#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Context {
public:
    friend class Instance;
    
    Context() = delete;
    static VkInstance getInstance();

private:
    static VkInstance instance;
};

} // namespace fw
