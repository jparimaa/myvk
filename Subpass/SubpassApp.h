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
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler sampler;
    fw::Camera camera;
    fw::CameraController cameraController;
    fw::Transformation trans;
    MatrixUBO ubo;
    fw::Buffer uniformBuffer;
    std::vector<RenderObject> renderObjects;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

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
