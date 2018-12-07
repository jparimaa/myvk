#include "MandelbrotApp.h"
#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Pipeline.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <array>
#include <iostream>

namespace
{
const std::string c_shaderFolder = SHADER_PATH;
const int c_width = 1024;
const int c_height = 1024;
const int c_bufferSize = sizeof(glm::vec4) * c_width * c_height;
const int c_workgroupSize = 32;
} // namespace

MandelbrotApp::~MandelbrotApp()
{
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_computePipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
}

bool MandelbrotApp::initialize()
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    createBuffer();
    createDescriptorSetLayout();
    createPipeline();
    createDescriptorSets();
    createCommandBuffers();

    fw::API::setRenderingEnabled(false);

    std::cout << "Compute queue: " << fw::Context::getComputeQueue() << "\n"
              << "Graphics queue: " << fw::Context::getGraphicsQueue() << "\n";

    return true;
}

void MandelbrotApp::postUpdate()
{
    fw::API::quitApplication();

    std::cout << "MandelbrotApp::postUpdate: Writing image...\n";

    void* mappedMemory = NULL;
    vkMapMemory(m_logicalDevice, m_storageBuffer.getMemory(), 0, c_bufferSize, 0, &mappedMemory);
    glm::vec4* vecMemory = (glm::vec4*)mappedMemory;

    std::vector<uint8_t> image;
    image.reserve(c_bufferSize);
    for (int i = 0; i < c_width * c_height; ++i)
    {
        image.push_back((uint8_t)(255.0f * (vecMemory[i].r)));
        image.push_back((uint8_t)(255.0f * (vecMemory[i].g)));
        image.push_back((uint8_t)(255.0f * (vecMemory[i].b)));
        image.push_back((uint8_t)(255.0f * (vecMemory[i].a)));
    }

    vkUnmapMemory(m_logicalDevice, m_storageBuffer.getMemory());

    std::string fileName = "output.png";
    stbi_write_png(fileName.c_str(), c_width, c_height, 4, image.data(), 0);
    std::cout << "Wrote file " << fileName << "\n";
}

void MandelbrotApp::createBuffer()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    m_storageBuffer.create(c_bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, uboProperties);
}

void MandelbrotApp::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding storageLayoutBinding{};
    storageLayoutBinding.binding = 0;
    storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageLayoutBinding.descriptorCount = 1;
    storageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    storageLayoutBinding.pImmutableSamplers = nullptr; // Optional

    std::vector<VkDescriptorSetLayoutBinding> bindings = {storageLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = fw::ui32size(bindings);
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void MandelbrotApp::createPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_descriptorSetLayout);
    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    VkPipelineShaderStageCreateInfo shaderStage = fw::Pipeline::getComputeShaderStageInfo(c_shaderFolder + "shader.comp.spv");

    fw::Cleaner cleaner([&shaderStage, this]() {
        vkDestroyShaderModule(m_logicalDevice, shaderStage.module, nullptr);
    });

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, NULL, &m_pipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStage;
    pipelineCreateInfo.layout = m_pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_computePipeline));
}

void MandelbrotApp::createDescriptorSets()
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_descriptorSet));

    VkWriteDescriptorSet descriptorWrite{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_storageBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = c_bufferSize;

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, 1, &descriptorWrite, 0, nullptr);
}

void MandelbrotApp::createCommandBuffers()
{
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = fw::API::getComputeCommandPool();
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &commandBufferAllocateInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, NULL);

    vkCmdDispatch(commandBuffer,
                  (uint32_t)ceil(c_width / float(c_workgroupSize)),
                  (uint32_t)ceil(c_height / float(c_workgroupSize)),
                  1);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    fw::API::setNextComputeCommandBuffer(commandBuffer);
}
