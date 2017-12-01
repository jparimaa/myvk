#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fw {

class Window {
   public:
    Window();
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    void initialize();
    bool shouldClose() const;
    void pollEvents() const;

   private:
    int width = 1600;
    int height = 1200;
    GLFWwindow* window = nullptr;
};

}  // namespace fw