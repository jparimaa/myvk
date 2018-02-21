#pragma once

#include "../Framework/Texture.h"
#include "../Framework/Sampler.h"
#include "../Framework/Image.h"
#include "../Framework/Buffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class EquirectangularHDR
{
public:
    struct Offscreen {        
        fw::Image image;
        VkImageView imageView;
        VkDeviceMemory memory;
        VkFramebuffer framebuffer;

        ~Offscreen();
    };
    
    EquirectangularHDR() {};
    ~EquirectangularHDR();
    EquirectangularHDR(const EquirectangularHDR&) = delete;
    EquirectangularHDR(EquirectangularHDR&&) = delete;
    EquirectangularHDR& operator=(const EquirectangularHDR&) = delete;
    EquirectangularHDR& operator=(EquirectangularHDR&&) = delete;

    bool initialize(const std::string& filename);
    VkImageView getImageView() const;

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
    int32_t size = 512;

    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    uint32_t numIndices;

    fw::Texture texture;
    fw::Sampler sampler;
    
    fw::Image image;
    VkImageView imageView = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    
    Offscreen offscreen;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    bool loadModel();
    bool createTargetImage();
    bool createRenderPass();
    bool createOffscreenFramebuffer();
    bool createDescriptors();
    bool createPipeline();
    void render();
    void draw(uint32_t face, VkCommandBuffer cmd);
};
