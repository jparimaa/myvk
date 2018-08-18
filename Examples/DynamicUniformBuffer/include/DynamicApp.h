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

class DynamicApp : public fw::Application
{
public:
    struct GlobalMatrices
    {
        glm::mat4 view;
        glm::mat4 proj;
    };

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

    DynamicApp(){};
    virtual ~DynamicApp();
    DynamicApp(const DynamicApp&) = delete;
    DynamicApp(DynamicApp&&) = delete;
    DynamicApp& operator=(const DynamicApp&) = delete;
    DynamicApp& operator=(DynamicApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;

private:
    VkDeviceSize m_minUniformBufferOffsetAlignment = 0;
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
    size_t m_dynamicAlignment = 0;
    size_t m_dynamicBufferSize = 0;
    glm::mat4* m_dynamicBufferData = nullptr;
    fw::Buffer m_dynamicBuffer;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkExtent2D m_extent;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObject();
    void createDescriptorSets();
    void createCommandBuffers();
};
