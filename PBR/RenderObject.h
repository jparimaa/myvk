#pragma once

#include "Helpers.h"

#include "../Framework/Sampler.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"
#include "../Framework/Texture.h"

#include <vulkan/vulkan.h>

class RenderObject
{
public:
    RenderObject() {};
    ~RenderObject();
    RenderObject(const RenderObject&) = delete;
    RenderObject(RenderObject&&) = delete;
    RenderObject& operator=(const RenderObject&) = delete;
    RenderObject& operator=(RenderObject&&) = delete;

    bool initialize(VkRenderPass pass, VkDescriptorPool pool, VkSampler textureSampler);
    void update(const fw::Camera& camera);
    void render(VkCommandBuffer cb);

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    uint32_t numIndices;
    fw::Texture texture;
    fw::Transformation transformation;
    fw::Buffer transformationBuffer;

    bool createDescriptorSetLayout();
    bool createPipeline();
    bool createRenderObject();
    bool allocateDescriptorSet();
    void updateDescriptorSet(VkImageView imageView);
};