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
    struct MatrixUBO
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct ObjectData
    {
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    struct RenderObject
    {
        fw::Transformation transformation;
        MatrixUBO matrices;
        fw::Buffer uniformBuffer;
        std::vector<ObjectData> objectData;
    };

    struct Subpass
    {
        ~Subpass();
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
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
    VkDescriptorSet m_compositeDescriptorSet;

    std::vector<FramebufferAttachment> m_framebufferAttachments;

    fw::Sampler m_sampler;
    fw::Camera m_camera;
    fw::CameraController m_cameraController;

    RenderObject m_droid;
    RenderObject m_cube;

    GBufferPass m_gbufferPass;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    void createGBufferAttachments();
    void createRenderPass();
    void createFramebuffers();
    void createDescriptorSetLayouts();
    void createGBufferPipeline();
    void createCompositePipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createAndUpdateCompositeDescriptorSet();
    void createCommandBuffers();
    void renderObject(VkCommandBuffer cb, const RenderObject& object, float reflectivity);
};
