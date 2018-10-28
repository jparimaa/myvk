#pragma once

#include "LightShaftCommon.h"

#include "fw/Image.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/Transformation.h"

#include <vulkan/vulkan.h>

#include <vector>

class PreLightShaft
{
public:
    PreLightShaft(){};
    ~PreLightShaft();
    bool initialize(uint32_t width, uint32_t height, VkDescriptorSetLayout matrixDescriptorSetLayout);
    void update(const fw::Camera& camera);
    void writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects);

    VkImageView getOutputImageView() const;
    const fw::Transformation& getLightTransformation() const;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    fw::Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    fw::Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_matrixDescriptorSetLayout = VK_NULL_HANDLE;
    fw::Buffer m_sphereMatrixBuffer;

    fw::Transformation m_sphereTransformation;
    MatrixUBO m_ubo;
    RenderObject m_sphere;

    void createFramebuffer();
    void createRenderPass();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSet();
    void createRenderObject();
};
