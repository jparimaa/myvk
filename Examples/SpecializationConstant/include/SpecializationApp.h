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

class SpecializationApp : public fw::Application
{
public:
    struct MatrixUBO
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct RenderObject
    {
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    SpecializationApp(){};
    virtual ~SpecializationApp();
    SpecializationApp(const SpecializationApp&) = delete;
    SpecializationApp(SpecializationApp&&) = delete;
    SpecializationApp& operator=(const SpecializationApp&) = delete;
    SpecializationApp& operator=(SpecializationApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;
    virtual void postUpdate() final{};

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_grayscalePipeline = VK_NULL_HANDLE;
    VkPipeline m_colorPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    fw::Transformation m_trans;
    MatrixUBO m_ubo;
    fw::Buffer m_uniformBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkExtent2D extent;

    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    void createCommandBuffers();
};
