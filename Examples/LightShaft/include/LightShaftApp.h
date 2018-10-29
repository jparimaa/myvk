#pragma once

#include "LightShaftCreator.h"
#include "LightShaftCommon.h"

#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>

#include <vector>

class LightShaftApp : public fw::Application
{
public:
    LightShaftApp(){};
    virtual ~LightShaftApp();
    LightShaftApp(const LightShaftApp&) = delete;
    LightShaftApp(LightShaftApp&&) = delete;
    LightShaftApp& operator=(const LightShaftApp&) = delete;
    LightShaftApp& operator=(LightShaftApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkExtent2D m_extent;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_matrixDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    fw::Transformation m_trans;
    MatrixUBO m_ubo;
    fw::Buffer m_uniformBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_matrixDescriptorSets;
    std::vector<VkDescriptorSet> m_textureDescriptorSets;

    VkCommandBuffer m_commandBuffer;
    LightShaftCreator m_lightShaftCreator;

    void createRenderPass();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorSet textureDescriptorSet, VkImageView imageView);
    void createCommandBuffer();
    void updateCommandBuffers();
};
