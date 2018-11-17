#pragma once

#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/Sampler.h"
#include "fw/Transformation.h"
#include "fw/Image.h"
#include "fw/Texture.h"

#include <vulkan/vulkan.h>

#include <vector>

class GBufferPass
{
public:
    GBufferPass(){};
    ~GBufferPass();

    void initialize(const fw::Camera* camera);
    void update();
    void writeRenderCommands(VkCommandBuffer cb);

private:
    struct RenderObject
    {
        fw::Buffer vertexBuffer;
        fw::Buffer indexBuffer;
        uint32_t numIndices;
        fw::Texture texture;
        VkDescriptorSet matrixDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
    };

    struct MatrixUBO
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct Attachment
    {
        fw::Image image;
        VkImageView imageView = VK_NULL_HANDLE;
    };

    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_matrixDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    const fw::Camera* m_camera = nullptr;

    fw::Transformation m_trans;
    MatrixUBO m_ubo;
    fw::Buffer m_uniformBuffer;
    std::vector<RenderObject> m_renderObjects;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_matrixDescriptorSets;
    std::vector<VkDescriptorSet> m_textureDescriptorSets;

    Attachment m_albedo;
    Attachment m_position;
    Attachment m_normal;
    Attachment m_depth;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    void createRenderPass();
    void createFramebuffer();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
    void createDescriptorSets(uint32_t setCount);
    void updateDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorSet textureDescriptorSet, VkImageView imageView);
};
