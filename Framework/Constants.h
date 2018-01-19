#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace fw::Constants
{

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 left = glm::vec3(-1.0f, 0.0f, 0.0f);

} // namespace fw
