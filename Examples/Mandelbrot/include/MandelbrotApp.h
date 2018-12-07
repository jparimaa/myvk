#pragma once

#include "fw/Application.h"
#include "fw/Buffer.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

class MandelbrotApp : public fw::Application
{
public:
    MandelbrotApp(){};
    virtual ~MandelbrotApp();
    MandelbrotApp(const MandelbrotApp&) = delete;
    MandelbrotApp(MandelbrotApp&&) = delete;
    MandelbrotApp& operator=(const MandelbrotApp&) = delete;
    MandelbrotApp& operator=(MandelbrotApp&&) = delete;

    virtual bool initialize() final;
    virtual void update() final{};
    virtual void onGUI() final{};
    virtual void postUpdate() final;

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_computePipeline = VK_NULL_HANDLE;

    fw::Buffer m_storageBuffer;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet;

    void createBuffer();
    void createDescriptorSetLayout();
    void createPipeline();
    void createDescriptorSets();
    void createCommandBuffers();
};
