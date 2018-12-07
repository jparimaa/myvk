#pragma once

#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"

#include <glm/glm.hpp>

#include <vector>

class TriangleApp : public fw::Application
{
public:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    TriangleApp(){};
    virtual ~TriangleApp();
    TriangleApp(const TriangleApp&) = delete;
    TriangleApp(TriangleApp&&) = delete;
    TriangleApp& operator=(const TriangleApp&) = delete;
    TriangleApp& operator=(TriangleApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final{};
    virtual void onGUI() final{};
    virtual void postUpdate() final{};

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    fw::Buffer m_vertexBuffer;
    fw::Buffer m_indexBuffer;
    uint32_t m_numIndices;
    fw::Texture m_texture;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSet();
    void createCommandBuffers();
};
