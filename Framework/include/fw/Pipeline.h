#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace fw
{
class Pipeline
{
public:
    Pipeline() = delete;
    static std::vector<VkPipelineShaderStageCreateInfo> getShaderStageInfos(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
    static VkPipelineShaderStageCreateInfo getComputeShaderStageInfo(const std::string& computeShaderFilename);
    static VkVertexInputBindingDescription getVertexDescription();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    static VkPipelineVertexInputStateCreateInfo
    getVertexInputState(const VkVertexInputBindingDescription* vertexDescription, const std::vector<VkVertexInputAttributeDescription>* attributeDescriptions);
    static VkPipelineInputAssemblyStateCreateInfo getInputAssemblyState();
    static VkViewport getViewport();
    static VkRect2D getScissorRect();
    static VkPipelineViewportStateCreateInfo getViewportState(const VkViewport* viewport, const VkRect2D* scissor);
    static VkPipelineRasterizationStateCreateInfo getRasterizationState();
    static VkPipelineMultisampleStateCreateInfo getMultisampleState();
    static VkPipelineDepthStencilStateCreateInfo getDepthStencilState();
    static VkPipelineColorBlendAttachmentState getColorBlendAttachmentState();
    static VkPipelineColorBlendStateCreateInfo
    getColorBlendState(const VkPipelineColorBlendAttachmentState* attachment);
    static VkPipelineLayoutCreateInfo getPipelineLayoutInfo(const VkDescriptorSetLayout* setLayout);
};

} // namespace fw
