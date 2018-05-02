#pragma once

#include "Helpers.h"

#include "../Framework/Sampler.h"
#include "../Framework/Buffer.h"
#include "../Framework/Transformation.h"
#include "../Framework/Camera.h"
#include "../Framework/Texture.h"

#include <vulkan/vulkan.h>
#include <assimp/material.h>

#include <unordered_map>

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
    void setImages(VkImageView irradiance, VkImageView prefilter, VkImageView brdf);
    void update(const fw::Camera& camera);
    void render(VkCommandBuffer cb);

private:
    struct TextureInfo
    {
        aiTextureType type;
        fw::Texture texture;
        uint32_t binding;
        VkImageView imageView;
    };

    struct UniformData
    {
        TransformMatrices transformationMatrices;
        glm::vec3 cameraPosition;
    };

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
    uint32_t numIndices = 0;
    std::vector<TextureInfo> textures;
    std::vector<TextureInfo> images;

    fw::Transformation transformation;
    UniformData uniformData;
    fw::Buffer uniformBuffer;

    bool createDescriptorSetLayout();
    bool createPipeline();
    bool createRenderObject();
    bool allocateDescriptorSet();
    void updateDescriptorSet();
};
