#pragma once

#include "Helpers.h"
#include "EnvironmentImages.h"
#include "Skybox.h"
#include "BRDFLUT.h"
#include "RenderObject.h"

#include "fw/Application.h"
#include "fw/Texture.h"
#include "fw/Sampler.h"
#include "fw/Buffer.h"
#include "fw/Transformation.h"
#include "fw/Camera.h"
#include "fw/CameraController.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

class PBRApp : public fw::Application
{
public:
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

    Skybox skybox;
    RenderObject renderObject;

    EnvironmentImages environmentImages;
    BRDFLUT brdfLut;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkExtent2D extent;

    void createRenderPass();
    void createDescriptorPool();
    void createCommandBuffers();
};
