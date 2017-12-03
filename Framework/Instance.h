#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Instance {
public:
    Instance();
    ~Instance();
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;

    bool initialize();

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT callback;

    bool createInstance();
    bool createDebugReportCallback();
};

} // namespace fw
