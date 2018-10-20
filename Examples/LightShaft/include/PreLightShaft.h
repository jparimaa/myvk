#pragma once

#include "LightShaftCommon.h"

#include "fw/Image.h"
#include "fw/Buffer.h"

#include <vulkan/vulkan.h>

#include <vector>

class PreLightShaft
{
public:
    PreLightShaft(){};
    ~PreLightShaft();
    bool init(uint32_t width, uint32_t height);
    void writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects);

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    fw::Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    fw::Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet;
    fw::Buffer m_uniformBuffer;

    void createFramebuffer();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSets();
};
