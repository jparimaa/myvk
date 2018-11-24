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

    VkImageView getAlbedoImageView() const;
    VkImageView getPositionImageView() const;
    VkImageView getNormalImageView() const;

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

    struct Attachment
    {
        fw::Image image;
        VkImageView imageView = VK_NULL_HANDLE;
    };

    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    fw::Sampler m_sampler;
    const fw::Camera* m_camera = nullptr;

    RenderObject m_droid;
    RenderObject m_cube;
    RenderObject m_wall;

    Attachment m_albedo;
    Attachment m_position;
    Attachment m_normal;
    Attachment m_depth;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    void renderObject(VkCommandBuffer cb, const RenderObject& object, float reflectivity);
    void createRenderPass();
    void createFramebuffer();
    void createDescriptorSetLayouts();
    void createPipeline();
    void createDescriptorPool();
    void createRenderObjects();
};
