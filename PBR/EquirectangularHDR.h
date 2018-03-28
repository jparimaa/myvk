#pragma once

#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Image.h"
#include "../Framework/Buffer.h"

#include <vulkan/vulkan.h>

#include <string>

class EquirectangularHDR
{
public:
    EquirectangularHDR() {};
    ~EquirectangularHDR();
    EquirectangularHDR(const EquirectangularHDR&) = delete;
    EquirectangularHDR(EquirectangularHDR&&) = delete;
    EquirectangularHDR& operator=(const EquirectangularHDR&) = delete;
    EquirectangularHDR& operator=(EquirectangularHDR&&) = delete;

    bool initialize(const std::string& filename);
    VkImageView getPlainImageView() const;
    VkImageView getIrradianceImageView() const;

private:
    struct Offscreen
    {
        fw::Image image;
        VkImageView imageView;
        VkFramebuffer framebuffer;
        uint32_t framebufferSize;

        ~Offscreen();
        bool createFramebuffer(VkRenderPass renderPass, uint32_t size, uint32_t layerCount, uint32_t levelCount);
    };

    struct Pipeline
    {
        struct PipelineParameters
        {
            VkDescriptorSetLayout descriptorSetLayout;
            uint32_t viewportSize;
            VkPushConstantRange pushConstantRange;
            std::string vertexShader;
            std::string fragmentShader;
            VkRenderPass renderPass;
        };
        
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDevice logicalDevice;
        PipelineParameters parameters;
        
        ~Pipeline();
        bool createPipeline(const PipelineParameters& params);
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
    uint32_t numIndices;

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
    void render(Offscreen& offscreen, Pipeline& pipeline, Target target);
};
