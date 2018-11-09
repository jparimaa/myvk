#pragma once

#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>

#include <vector>

class SecondaryApp : public fw::Application
{
public:
    struct RenderObject
    {
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
    };

    struct BufferObject
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        fw::Transformation trans;
        fw::Buffer uniformBuffer;
    };

    SecondaryApp();
    virtual ~SecondaryApp();
    SecondaryApp(const SecondaryApp&) = delete;
    SecondaryApp(SecondaryApp&&) = delete;
    SecondaryApp& operator=(const SecondaryApp&) = delete;
    SecondaryApp& operator=(SecondaryApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    RenderObject m_renderObject;
    std::vector<BufferObject> m_bufferObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkFence m_renderBufferFence;
    VkCommandBuffer m_primaryCommandBuffer;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;
    size_t m_currentFrameIndex = 0;

    VkExtent2D m_extent;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObject();
    void createDescriptorSets();
    void createCommandBuffers();
    void createFence();
    void updateCommandBuffers();
};
