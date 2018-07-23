#pragma once

#include "Offscreen.h"
#include "PipelineHelper.h"

#include "fw/Texture.h"
#include "fw/Sampler.h"
#include "fw/Image.h"
#include "fw/Buffer.h"

#include <glm/glm.hpp>
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

    void initialize(const std::string& filename);
    VkImageView getPlainImageView() const;
    VkImageView getIrradianceImageView() const;
    VkImageView getPrefilterImageView() const;

private:
    struct PlainPushConstants
    {
        glm::mat4 mvp;
    };

    struct IrradiancePushConstants
    {
        glm::mat4 mvp;
        float deltaPhi = 0.0f;
        float deltaTheta = 0.0f;
    };

    struct PrefilterPushConstants
    {
        glm::mat4 mvp;
        float roughness = 0.0f;
        uint32_t numSamples = 32u;
    };

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
    uint32_t prefilterLevelCount = 0;

    fw::Texture texture;
    fw::Sampler sampler;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    fw::Image plainImage;
    VkImageView plainImageView = VK_NULL_HANDLE;
    PlainPushConstants plainPushConstants;

    fw::Image irradianceImage;
    VkImageView irradianceImageView = VK_NULL_HANDLE;
    IrradiancePushConstants irradiancePushConstants;

    fw::Image prefilterImage;
    VkImageView prefilterImageView = VK_NULL_HANDLE;
    PrefilterPushConstants prefilterPushConstants;

    void loadModel();
    void createCubeImage(uint32_t size, uint32_t mipLevels, fw::Image& image, VkImageView& imageView);
    void createRenderPass();
    void createDescriptors();
    void createEnvironmentImage(int32_t textureSize, VkPushConstantRange range,
                                const std::string& shader, VkImageView input, Target target);
    void updateDescriptors(VkImageView imageView);
    fw::Image& getImageByTarget(Target target);
    uint32_t getLevelCountByTarget(Target target);
    void render(Offscreen& offscreen, PipelineHelper& pipelineHelper, Target target);
    void changeLayoutToShaderRead(Target target);
};
