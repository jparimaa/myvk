#pragma once

#include <vulkan/vulkan.h>

#include <string>

class PipelineHelper
{
public:
    static VkDescriptorSetLayout descriptorSetLayout;
    static VkRenderPass renderPass;

    PipelineHelper() {};
    ~PipelineHelper();
    bool createPipeline(uint32_t viewportSize, VkPushConstantRange pushConstRange, const std::string& vertexShader, const std::string& fragmentShader);

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    VkPushConstantRange getPushConstantRange() const;

private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkPushConstantRange pushConstantRange;
};
