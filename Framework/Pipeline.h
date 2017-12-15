#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

namespace fw {

class Pipeline {
public:
    Pipeline() = delete;
    static std::vector<VkPipelineShaderStageCreateInfo> getDefaultShaderStageInfos(
        const std::string& vertexShaderFilename,
        const std::string& fragmentShaderFilename);
    static VkPipelineVertexInputStateCreateInfo getDefaultVertexInputInfo();
    static VkPipelineInputAssemblyStateCreateInfo getDefaultInputAssemblyInfo();
};

} // namespace fw
