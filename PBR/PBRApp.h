#pragma once

#include "EnvironmentImages.h"
#include "BRDFLUT.h"

#include "../Framework/Application.h"
#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"
#include "../Framework/CameraController.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

class PBRApp : public fw::Application
{
public:
    struct TransformMatrices
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct RenderObject
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        fw::Transformation transformation;
        fw::Buffer transformationBuffer;
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
    };

    PBRApp() {};
    virtual ~PBRApp();
    PBRApp(const PBRApp&) = delete;
    PBRApp(PBRApp&&) = delete;
    PBRApp& operator=(const PBRApp&) = delete;
    PBRApp& operator=(PBRApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void onGUI() final;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline skyboxPipeline = VK_NULL_HANDLE;

    fw::Sampler sampler;
    fw::Camera camera;
    fw::CameraController cameraController;

    RenderObject skybox{};

    EnvironmentImages environmentImages;
    BRDFLUT brdfLut;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkExtent2D extent;

    bool createRenderPass();
    bool createDescriptorSetLayout();
    bool createSkyboxPipeline();
    bool createDescriptorPool();
    bool createSkybox();
    bool createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    bool createCommandBuffers();
};
