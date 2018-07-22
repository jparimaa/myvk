#pragma once

#include "Helpers.h"

#include "../Framework/Sampler.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"

#include <vulkan/vulkan.h>

class Skybox
{
public:
    Skybox() {};
    ~Skybox();
    Skybox(const Skybox&) = delete;
    Skybox(Skybox&&) = delete;
    Skybox& operator=(const Skybox&) = delete;
    Skybox& operator=(Skybox&&) = delete;

    void initialize(VkRenderPass pass, VkDescriptorPool pool, VkSampler skyboxSampler, VkImageView skyboxTexture);
    void update(const fw::Camera& camera);
    void render(VkCommandBuffer cb);

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkRenderPass renderPass;
    VkSampler sampler;
    VkImageView texture;

    fw::Transformation transformation;
    fw::Buffer transformationBuffer;
    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    uint32_t numIndices = 0;

    void createDescriptorSetLayout();
    void createPipeline();
    void createSkybox();
};
