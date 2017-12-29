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
    static VkPipelineVertexInputStateCreateInfo getDefaultVertexInputState(
        const VkVertexInputBindingDescription* vertexDescription,
        const std::vector<VkVertexInputAttributeDescription>* attributeDescriptions);
    static VkPipelineInputAssemblyStateCreateInfo getDefaultInputAssemblyState();
    static VkViewport getDefaultViewport();
    static VkRect2D getDefaultScissorRect();
    static VkPipelineViewportStateCreateInfo getDefaultViewportState(const VkViewport* viewport, const VkRect2D* scissor);
    static VkPipelineRasterizationStateCreateInfo getDefaultRasterizationState();
    static VkPipelineMultisampleStateCreateInfo getDefaultMultisampleState();
    static VkPipelineDepthStencilStateCreateInfo getDefaultDepthStencilState();
    static VkPipelineColorBlendAttachmentState getDefaultColorBlendState();
    static VkPipelineColorBlendStateCreateInfo getDefaultColorBlendInfo(const VkPipelineColorBlendAttachmentState* colorBlendState);
    static VkPipelineLayoutCreateInfo getDefaultPipelineLayoutInfo(const VkDescriptorSetLayout* setLayout);
};

} // namespace fw
