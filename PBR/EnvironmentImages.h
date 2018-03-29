#pragma once

#include "Offscreen.h"
#include "PipelineHelper.h"

#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Image.h"
#include "../Framework/Buffer.h"

#include <vulkan/vulkan.h>

#include <string>

class EnvironmentImages
{
public:
    EnvironmentImages() {};
    ~EnvironmentImages();
    EnvironmentImages(const EnvironmentImages&) = delete;
    EnvironmentImages(EnvironmentImages&&) = delete;
    EnvironmentImages& operator=(const EnvironmentImages&) = delete;
    EnvironmentImages& operator=(EnvironmentImages&&) = delete;

    bool initialize(const std::string& filename);
    VkImageView getPlainImageView() const;
    VkImageView getIrradianceImageView() const;

private:
    enum class Target
    {
        plain = 0,
        irradiance = 1,
        prefilter = 2
    };

    VkDevice logicalDevice = VK_NULL_HANDLE;

    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    uint32_t numIndices = 0;

    fw::Texture texture;
    fw::Sampler sampler;
    
    fw::Image plainImage;
    VkImageView plainImageView = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    fw::Image irradianceImage;
    VkImageView irradianceImageView = VK_NULL_HANDLE;

    fw::Image prefilterImage;
    VkImageView prefilterImageView = VK_NULL_HANDLE;
    
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    bool loadModel();
    bool createCubeImage(uint32_t size, uint32_t mipLevels, fw::Image& image, VkImageView& imageView);
    bool createRenderPass();
    bool createDescriptors();  

    void updateDescriptors(VkImageView imageView);
    void render(Offscreen& offscreen, PipelineHelper& pipelineHelper, Target target);
};
