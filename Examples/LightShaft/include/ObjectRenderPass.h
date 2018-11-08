#pragma once

#include "LightShaftCommon.h"

#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/Sampler.h"
#include "fw/Transformation.h"

#include <vulkan/vulkan.h>

#include <vector>

class ObjectRenderPass
{
public:
    ObjectRenderPass(){};
    ~ObjectRenderPass();

    void initialize(const fw::Camera* camera);
    void update();
    void writeRenderCommands(VkCommandBuffer cb);

    VkDescriptorSetLayout getMatrixDescriptorSetLayout();
    const std::vector<RenderObject>& getRenderObjects();
    VkImageView getOutputImageView() const;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_matrixDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    const fw::Camera* m_camera = nullptr;

    fw::Transformation m_trans;
    MatrixUBO m_ubo;
    fw::Buffer m_uniformBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_matrixDescriptorSets;
    std::vector<VkDescriptorSet> m_textureDescriptorSets;

    fw::Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    fw::Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    void createRenderPass();
    void createFramebuffer();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorSet textureDescriptorSet, VkImageView imageView);
};
