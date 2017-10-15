#ifndef TESTAPP_H
#define TESTAPP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class TestApp {
 public:
  void run();

 private:
  GLFWwindow* window;
  VkInstance instance;

  void init();
  void cleanup();
};

#endif