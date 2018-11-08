#pragma once

#include "LightShaftCommon.h"

#include "fw/Image.h"
#include "fw/Buffer.h"
#include "fw/Camera.h"
#include "fw/Transformation.h"

#include <vulkan/vulkan.h>

#include <vector>

class LightShaftPrepass
{
public:
    LightShaftPrepass(){};
    ~LightShaftPrepass();
    bool initialize(VkDescriptorSetLayout matrixDescriptorSetLayout, const fw::Transformation* light);
    void update(const fw::Camera& camera);
    void writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects);

    VkImageView getOutputImageView() const;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    fw::Image m_image;
    VkImageView m_imageView = VK_NULL_HANDLE;
    fw::Image m_depthImage;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_matrixDescriptorSetLayout = VK_NULL_HANDLE;
    fw::Buffer m_sphereMatrixBuffer;

    const fw::Transformation* m_sphereTransformation = nullptr;

    MatrixUBO m_ubo;
    RenderObject m_sphere;

    void createRenderPass();
    void createFramebuffer();
    void createPipeline();
    void createDescriptorPool();
    void createDescriptorSet();
    void createRenderObject();
};
