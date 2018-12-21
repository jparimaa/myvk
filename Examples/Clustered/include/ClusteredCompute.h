#pragma once

#include "fw/Buffer.h"

#include <vulkan/vulkan.h>

class ClusteredCompute
{
public:
    ClusteredCompute(){};
    ~ClusteredCompute();

    bool initialize(fw::Buffer* storageBuffer);

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_cullingPipeline = VK_NULL_HANDLE;

    fw::Buffer* m_storageBuffer;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet;

    void writeRandomData();
    void createDescriptorSetLayout();
    void createCullingPipeline();
    void createDescriptorSets();
    void createCommandBuffers();
};
