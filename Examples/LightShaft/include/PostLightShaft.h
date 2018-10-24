#pragma once

#include "fw/Image.h"
#include "fw/Buffer.h"

#include <vulkan/vulkan.h>

class PostLightShaft
{
public:
    PostLightShaft(){};
    ~PostLightShaft();
    bool initialize(uint32_t width, uint32_t height);
    void writeRenderCommands(VkCommandBuffer cb);

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    fw::Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    void createFramebuffer();
    void createRenderPass();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSet();
};
