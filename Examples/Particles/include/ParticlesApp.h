#pragma once

#include "Helpers.h"
#include "ParticleCompute.h"

#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"

#include <glm/glm.hpp>

#include <vector>

class ParticlesApp : public fw::Application
{
public:
    ParticlesApp(){};
    virtual ~ParticlesApp();
    ParticlesApp(const ParticlesApp&) = delete;
    ParticlesApp(ParticlesApp&&) = delete;
    ParticlesApp& operator=(const ParticlesApp&) = delete;
    ParticlesApp& operator=(ParticlesApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;
    virtual void postUpdate() final{};

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    Matrices m_matrices;
    fw::Buffer m_uniformBuffer;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet;

    ParticleCompute m_particleCompute;
    fw::Buffer m_storageBuffer;

    void createBuffer();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
};
