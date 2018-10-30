#pragma once

#include "fw/Image.h"
#include "fw/Buffer.h"
#include "fw/Sampler.h"
#include "fw/Camera.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class PostLightShaft
{
public:
    PostLightShaft(){};
    ~PostLightShaft();
    bool initialize(VkImageView inputImageView);
    void update(const fw::Camera& camera, const fw::Transformation& light);
    void writeRenderCommands(VkCommandBuffer cb, VkFramebuffer finalFramebuffer);

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;

    fw::Sampler m_sampler;

    VkImageView m_inputImageView = VK_NULL_HANDLE;
    glm::vec2 m_lightPosScreen;

    void createRenderPass();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSet();
};
