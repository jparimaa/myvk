#pragma once

#include "Helpers.h"
#include "ClusteredCompute.h"

#include "fw/Application.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"
#include "fw/Transformation.h"

#include <glm/glm.hpp>

#include <vector>

class ClusteredApp : public fw::Application
{
public:
    struct RenderObject
    {
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    ClusteredApp(){};
    virtual ~ClusteredApp();
    ClusteredApp(const ClusteredApp&) = delete;
    ClusteredApp(ClusteredApp&&) = delete;
    ClusteredApp& operator=(const ClusteredApp&) = delete;
    ClusteredApp& operator=(ClusteredApp&&) = delete;

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

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    fw::Transformation m_transformation;
    Matrices m_matrices;
    fw::Buffer m_matrixBuffer;
    fw::Buffer m_sceneBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;

    ClusteredCompute m_clusteredCompute;
    fw::Buffer m_lightStorageBuffer;
    fw::Buffer m_tileStorageBuffer;

    void createBuffers();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    void createCommandBuffers();
};
