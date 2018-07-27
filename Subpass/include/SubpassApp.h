#pragma once

#include "fw/Application.h"
#include "fw/Texture.h"
#include "fw/Sampler.h"
#include "fw/Buffer.h"
#include "fw/Transformation.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"

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
        ~Subpass();
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct FramebufferAttachment
    {
        ~FramebufferAttachment();
        fw::Image image;
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };


    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_framebuffers;

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

    void createGBufferAttachments();
    void createRenderPass();
    void createFramebuffers();
    void createDescriptorSetLayouts();
    void createGBufferPipeline();
    void createCompositePipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createGBufferDescriptorSets(uint32_t setCount);
    void updateGBufferDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    void createAndUpdateCompositeDescriptorSet();
    void createCommandBuffers();
};
