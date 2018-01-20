#pragma once

#include "../Framework/Application.h"
#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Model.h"
#include "../Framework/Buffer.h"

#include <vector>

class ExampleApp : public fw::Application
{
public:
    ExampleApp();
    virtual ~ExampleApp();
    ExampleApp(const ExampleApp&) = delete;
    ExampleApp(ExampleApp&&) = delete;
    ExampleApp& operator=(const ExampleApp&) = delete;
    ExampleApp& operator=(ExampleApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final;
    virtual void render() final;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    fw::Texture texture;
    fw::Sampler sampler;
    fw::Buffer uniformBuffer;
    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers;
    
    bool createRenderPass();
    bool createDescriptorSetLayout();
    bool createPipeline();
    bool createBuffers();
    bool createDescriptorPool();
    bool createDescriptorSet();
    bool createCommandBuffers();
};
