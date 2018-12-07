#pragma once

#include "LightShaftPrepass.h"
#include "ObjectRenderPass.h"

#include "fw/Application.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"
#include "fw/Sampler.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

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
    virtual void postUpdate() final{};

private:
    struct ShaderParameters
    {
        glm::vec2 lightPosScreen;
        int numSamples = 100;
        float density = 1.0f;
        float weight = 0.02f;
        float decay = 0.99f;
        float exposure = 1.0f;
    };

    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkCommandBuffer m_commandBuffer;
    VkFence m_renderBufferFence;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;

    fw::Transformation m_lightTransformation;

    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;

    VkImageView m_inputImageView = VK_NULL_HANDLE;
    ShaderParameters m_shaderParameters;

    ObjectRenderPass m_objectRenderPass;
    LightShaftPrepass m_lightShaftPrepass;

    void createRenderPass();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffer();
    void createFence();
    void updateCommandBuffers();
};
