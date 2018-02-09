#pragma once

#include "../Framework/Application.h"
#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Model.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"
#include "../Framework/CameraController.h"
#include "../Framework/GUI.h"

#include <glm/glm.hpp>

#include <vector>

class ExampleApp : public fw::Application
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
    
    ExampleApp();
    virtual ~ExampleApp();
    ExampleApp(const ExampleApp&) = delete;
    ExampleApp(ExampleApp&&) = delete;
    ExampleApp& operator=(const ExampleApp&) = delete;
    ExampleApp& operator=(ExampleApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;

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
    fw::GUI gui;
    
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    VkExtent2D extent;
    
    bool createRenderPass();
    bool createDescriptorSetLayout();
    bool createPipeline();
    bool createDescriptorPool();
    bool createRenderObjects();
    bool createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkImageView imageView);
    bool createCommandBuffers();
    void createGUI(VkCommandBuffer commandBuffer);
};
