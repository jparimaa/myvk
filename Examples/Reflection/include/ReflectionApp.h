#pragma once

#include "GBufferPass.h"
#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

class ReflectionApp : public fw::Application
{
public:
    ReflectionApp(){};
    virtual ~ReflectionApp();
    ReflectionApp(const ReflectionApp&) = delete;
    ReflectionApp(ReflectionApp&&) = delete;
    ReflectionApp& operator=(const ReflectionApp&) = delete;
    ReflectionApp& operator=(ReflectionApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final{};

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;

    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;

    GBufferPass m_gbufferPass;

    fw::Buffer m_projectionMatrixUniformBuffer;

    void createRenderPass();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
};
