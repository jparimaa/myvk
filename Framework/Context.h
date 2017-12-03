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

    const VkInstance* getInstance() const;

   private:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT callback;

    bool createInstance();
    bool createDebugReportCallback();
};

} // namespace fw
