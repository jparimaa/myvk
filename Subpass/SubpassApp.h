#pragma once

#include "../Framework/Application.h"
#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"
#include "../Framework/CameraController.h"

#include <glm/glm.hpp>

#include <vector>

class SubpassApp : public fw::Application
{
public:
    SubpassApp() {};
    virtual ~SubpassApp();
    SubpassApp(const SubpassApp&) = delete;
    SubpassApp(SubpassApp&&) = delete;
    SubpassApp& operator=(const SubpassApp&) = delete;
    SubpassApp& operator=(SubpassApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final {};

private:
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

    struct Subpass
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct FramebufferAttachment
    {
        fw::Image image;
        VkImageView imageView = VK_NULL_HANDLE;
    };


    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    Subpass m_gbuffer;
    Subpass m_composite;

    std::vector<FramebufferAttachment> m_framebufferAttachments;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;
    fw::Transformation m_transformation;
    MatrixUBO m_ubo;
    fw::Buffer m_uniformBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    void createRenderPass();
    void createDescriptorSetLayouts();
    void createGBufferPipeline();
    void createCompositePipeline();
    void createDescriptorPool();
    void createGBufferAttachments();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    void createCommandBuffers();
};
