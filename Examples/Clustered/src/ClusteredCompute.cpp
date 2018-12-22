#include "ClusteredCompute.h"
#include "Helpers.h"

#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Pipeline.h"

#include <array>
#include <random>
#include <iostream>

ClusteredCompute::~ClusteredCompute()
{
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_cullingPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
}

bool ClusteredCompute::initialize(fw::Buffer* lightBuffer, fw::Buffer* lightIndexBuffer, fw::Buffer* tileBuffer)
{
    m_logicalDevice = fw::Context::getLogicalDevice();
    m_lightStorageBuffer = lightBuffer;
    m_lightIndexStorageBuffer = lightIndexBuffer;
    m_tileStorageBuffer = tileBuffer;

    writeRandomData();
    createDescriptorSetLayout();
    createCullingPipeline();
    createDescriptorSets();
    createCommandBuffers();

    return true;
}

void ClusteredCompute::writeRandomData()
{
    void* mappedMemory = NULL;
    vkMapMemory(m_logicalDevice, m_lightStorageBuffer->getMemory(), 0, c_lightBufferSize, 0, &mappedMemory);
    Light* lightMemory = (Light*)mappedMemory;

    std::default_random_engine randomEngine;
    std::uniform_real_distribution<float> positionDistribution(-10.0f, 10.0f);
    std::uniform_real_distribution<float> radiusDistribution(0.5f, 2.0f);
    std::uniform_real_distribution<float> colorDistribution(0.0f, 1.0f);
    for (int i = 0; i < c_numLights; ++i)
    {
        glm::vec3 position(positionDistribution(randomEngine),
                           positionDistribution(randomEngine),
                           positionDistribution(randomEngine));
        float radius = radiusDistribution(randomEngine);
        lightMemory[i].position = glm::vec4(position.x, position.y, position.z, radius);

        glm::vec3 color(colorDistribution(randomEngine),
                        colorDistribution(randomEngine),
                        colorDistribution(randomEngine));
        float power = 1.0f;
        lightMemory[i].color = glm::vec4(color.x, color.y, color.z, power);
    }

    vkUnmapMemory(m_logicalDevice, m_lightStorageBuffer->getMemory());
}

void ClusteredCompute::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding lightStorageBinding{};
    lightStorageBinding.binding = 0;
    lightStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightStorageBinding.descriptorCount = 1;
    lightStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    lightStorageBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding lightIndexStorageBinding{};
    lightIndexStorageBinding.binding = 1;
    lightIndexStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightIndexStorageBinding.descriptorCount = 1;
    lightIndexStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    lightIndexStorageBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding tileStorageBinding{};
    tileStorageBinding.binding = 2;
    tileStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    tileStorageBinding.descriptorCount = 1;
    tileStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    tileStorageBinding.pImmutableSamplers = nullptr; // Optional

    std::vector<VkDescriptorSetLayoutBinding> bindings = {lightStorageBinding, lightIndexStorageBinding, tileStorageBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = fw::ui32size(bindings);
    layoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void ClusteredCompute::createCullingPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_descriptorSetLayout);
    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    VkPipelineShaderStageCreateInfo shaderStage = fw::Pipeline::getComputeShaderStageInfo(c_shaderFolder + "culling.comp.spv");

    fw::Cleaner cleaner([&shaderStage, this]() {
        vkDestroyShaderModule(m_logicalDevice, shaderStage.module, nullptr);
    });

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStage;
    pipelineCreateInfo.layout = m_pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_cullingPipeline));
}

void ClusteredCompute::createDescriptorSets()
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &m_descriptorSet));

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    VkDescriptorBufferInfo lightBufferInfo{};
    lightBufferInfo.buffer = m_lightStorageBuffer->getBuffer();
    lightBufferInfo.offset = 0;
    lightBufferInfo.range = c_lightBufferSize;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &lightBufferInfo;

    VkDescriptorBufferInfo lightIndexBufferInfo{};
    lightIndexBufferInfo.buffer = m_lightIndexStorageBuffer->getBuffer();
    lightIndexBufferInfo.offset = 0;
    lightIndexBufferInfo.range = c_lightIndexBufferSize;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &lightIndexBufferInfo;

    VkDescriptorBufferInfo tileBufferInfo{};
    tileBufferInfo.buffer = m_tileStorageBuffer->getBuffer();
    tileBufferInfo.offset = 0;
    tileBufferInfo.range = c_tileBufferSize;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_descriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &tileBufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void ClusteredCompute::createCommandBuffers()
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
    beginInfo.flags = 0;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkBufferMemoryBarrier bufferBarrier{};
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferBarrier.buffer = m_lightStorageBuffer->getBuffer();
    bufferBarrier.size = c_lightBufferSize;
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Rendering invocations have finished reading from the buffer
    bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Compute shader wants to write to the buffer
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         1,
                         &bufferBarrier,
                         0,
                         nullptr);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_cullingPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, NULL);
    vkCmdDispatch(commandBuffer, 1, 1, c_gridDepth);

    // Add memory barrier to ensure that compute shader has finished writing to the buffer
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Compute shader has finished writes to the buffer
    bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bufferBarrier.buffer = m_lightStorageBuffer->getBuffer();
    bufferBarrier.size = c_lightBufferSize;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        1,
        &bufferBarrier,
        0,
        nullptr);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    fw::API::setNextComputeCommandBuffer(commandBuffer);
}
