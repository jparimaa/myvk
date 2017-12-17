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
    static VkVertexInputBindingDescription getDefaultVertexDescription();
    static std::vector<VkVertexInputAttributeDescription> getDefaultAttributeDescriptions();
    static VkPipelineVertexInputStateCreateInfo getDefaultVertexInputInfo(
        const VkVertexInputBindingDescription* vertexDescription,
        const std::vector<VkVertexInputAttributeDescription>* attributeDescriptions);
    static VkPipelineInputAssemblyStateCreateInfo getDefaultInputAssemblyInfo();
    static VkViewport getDefaultViewport();
    static VkRect2D getDefaultScissorRect();
    static VkPipelineViewportStateCreateInfo getDefaultViewportState(const VkViewport* viewport, const VkRect2D* scissor);
    static VkPipelineRasterizationStateCreateInfo getDefaultRasterizationInfo();
    static VkPipelineMultisampleStateCreateInfo getDefaultMultisampleInfo();
    static VkPipelineDepthStencilStateCreateInfo getDefaultDepthStencilInfo();
    static VkPipelineColorBlendAttachmentState getDefaultColorBlendState();
    static VkPipelineColorBlendStateCreateInfo getDefaultColorBlendInfo(const VkPipelineColorBlendAttachmentState* colorBlendState);
};

} // namespace fw
