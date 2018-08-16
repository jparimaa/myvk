#pragma once

#include "fw/Image.h"
#include "fw/Sampler.h"
#include "fw/Texture.h"

#include <vulkan/vulkan.h>

class BRDFLUT
{
public:
    BRDFLUT(){};
    ~BRDFLUT();

    bool initialize();
    VkImageView getImageView() const;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;

    fw::Sampler sampler;
    fw::Image image;
    VkImageView imageView = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    bool createImage();
    bool createRenderPass();
    bool createFramebuffer();
    bool createPipeline();
    void render();
};
