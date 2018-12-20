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
    vkDestroyPipeline(m_logicalDevice, m_positionPipeline, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_directionPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
}

bool ClusteredCompute::initialize(fw::Buffer* storageBuffer)
{
    m_logicalDevice = fw::Context::getLogicalDevice();
    m_storageBuffer = storageBuffer;

    writeRandomData();
    createDescriptorSetLayout();
    createDirectionPipeline();
    createPositionPipeline();
    createDescriptorSets();
    createCommandBuffers();

    return true;
}

void ClusteredCompute::writeRandomData()
{
    void* mappedMemory = NULL;
    vkMapMemory(m_logicalDevice, m_storageBuffer->getMemory(), 0, c_bufferSize, 0, &mappedMemory);
    Particle* particleMemory = (Particle*)mappedMemory;

    std::default_random_engine randomEngine;
    std::uniform_real_distribution<float> positionDistribution(-0.5f, 0.5f);
    std::uniform_real_distribution<float> scaleDistribution(0.8f, 1.0f);
    std::uniform_real_distribution<float> directionDistribution(1.0f, 2.0f);
    for (int i = 0; i < c_numParticles; ++i)
    {
        glm::vec3 p(positionDistribution(randomEngine),
                    positionDistribution(randomEngine),
                    positionDistribution(randomEngine));
        p = glm::normalize(p);
        p *= scaleDistribution(randomEngine);
        glm::vec4 position = glm::vec4(p.x, p.y, p.z, 1.0f);
        particleMemory[i].position = position;
        particleMemory[i].direction = position * c_initialSpeed;
    }

    vkUnmapMemory(m_logicalDevice, m_storageBuffer->getMemory());
}

void ClusteredCompute::createDescriptorSetLayout()
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

void ClusteredCompute::createDirectionPipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_descriptorSetLayout);
    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    VkPipelineShaderStageCreateInfo shaderStage = fw::Pipeline::getComputeShaderStageInfo(c_shaderFolder + "direction.comp.spv");

    fw::Cleaner cleaner([&shaderStage, this]() {
        vkDestroyShaderModule(m_logicalDevice, shaderStage.module, nullptr);
    });

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStage;
    pipelineCreateInfo.layout = m_pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_directionPipeline));
}

void ClusteredCompute::createPositionPipeline()
{
    VkPipelineShaderStageCreateInfo shaderStage = fw::Pipeline::getComputeShaderStageInfo(c_shaderFolder + "position.comp.spv");

    fw::Cleaner cleaner([&shaderStage, this]() {
        vkDestroyShaderModule(m_logicalDevice, shaderStage.module, nullptr);
    });

    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStage;
    pipelineCreateInfo.layout = m_pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_positionPipeline));
}

void ClusteredCompute::createDescriptorSets()
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
    bufferInfo.buffer = m_storageBuffer->getBuffer();
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
    bufferBarrier.buffer = m_storageBuffer->getBuffer();
    bufferBarrier.size = c_bufferSize;
    bufferBarrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; // Vertex shader invocations have finished reading from the buffer
    bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Compute shader wants to write to the buffer
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         1,
                         &bufferBarrier,
                         0,
                         nullptr);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_directionPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, NULL);
    vkCmdDispatch(commandBuffer, c_numParticles / c_workgroupSize, 1, 1);

    // Add memory barrier to ensure that compute shader has finished writing to the buffer
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Compute shader has finished writes to the buffer
    bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bufferBarrier.buffer = m_storageBuffer->getBuffer();
    bufferBarrier.size = c_bufferSize;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0,
        nullptr,
        1,
        &bufferBarrier,
        0,
        nullptr);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_positionPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, NULL);
    vkCmdDispatch(commandBuffer, c_numParticles / c_workgroupSize, 1, 1);

    // Add memory barrier to ensure that compute shader has finished writing to the buffer
    // Without this the (rendering) vertex shader may display incomplete results (partial data from last frame)
    bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Compute shader has finished writes to the buffer
    bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT; // Vertex shader invocations want to read from the buffer
    bufferBarrier.buffer = m_storageBuffer->getBuffer();
    bufferBarrier.size = c_bufferSize;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
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
