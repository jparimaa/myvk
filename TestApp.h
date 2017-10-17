#ifndef TESTAPP_H
#define TESTAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class TestApp {
 public:
  ~TestApp();
  void run();

 private:
  GLFWwindow* window = nullptr;
  VkInstance instance;
  VkDebugReportCallbackEXT callback;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  void init();
  void createWindow();
  void createInstance();
  void createDebugReportCallback();
  void getPhysicalDevice();
};

#endif