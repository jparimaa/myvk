#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Context {
   public:
    Context();
    ~Context();
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    bool initialize();

   private:
    VkInstance instance = nullptr;
    VkDebugReportCallbackEXT callback = nullptr;

    bool createInstance();
    bool createDebugReportCallback();
};

} // namespace fw
