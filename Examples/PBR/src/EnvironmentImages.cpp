#include "EnvironmentImages.h"
#include "Helpers.h"

#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Constants.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Model.h"
#include "fw/Pipeline.h"
#include "fw/RenderPass.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace
{
const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
const int32_t defaultSize = 512;
const int32_t irradianceSize = 64;

const glm::mat4 viewMatrices[] = {glm::lookAt(fw::Constants::zeroVec3, fw::Constants::right, fw::Constants::up),
                                  glm::lookAt(fw::Constants::zeroVec3, fw::Constants::left, fw::Constants::up),
                                  glm::lookAt(fw::Constants::zeroVec3, fw::Constants::down, fw::Constants::forward),
                                  glm::lookAt(fw::Constants::zeroVec3, fw::Constants::up, fw::Constants::backward),
                                  glm::lookAt(fw::Constants::zeroVec3, fw::Constants::forward, fw::Constants::up),
                                  glm::lookAt(fw::Constants::zeroVec3, fw::Constants::backward, fw::Constants::up)};

const glm::mat4 invertViewMatrices[]
    = {glm::lookAt(fw::Constants::zeroVec3, fw::Constants::right, fw::Constants::down),
       glm::lookAt(fw::Constants::zeroVec3, fw::Constants::left, fw::Constants::down),
       glm::lookAt(fw::Constants::zeroVec3, fw::Constants::up, fw::Constants::backward),
       glm::lookAt(fw::Constants::zeroVec3, fw::Constants::down, fw::Constants::forward),
       glm::lookAt(fw::Constants::zeroVec3, fw::Constants::backward, fw::Constants::down),
       glm::lookAt(fw::Constants::zeroVec3, fw::Constants::forward, fw::Constants::down)};

} // unnamed

EnvironmentImages::~EnvironmentImages()
{
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
    vkDestroyImageView(logicalDevice, plainImageView, nullptr);
    vkDestroyImageView(logicalDevice, irradianceImageView, nullptr);
    vkDestroyImageView(logicalDevice, prefilterImageView, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
}

void EnvironmentImages::initialize(const std::string& filename)
{
    logicalDevice = fw::Context::getLogicalDevice();
    prefilterLevelCount = static_cast<uint32_t>(floor(log2(defaultSize))) + 1;

    texture.load(filename, VK_FORMAT_R8G8B8A8_UNORM);
    loadModel();
    sampler.create(VK_COMPARE_OP_NEVER);
    createCubeImage(defaultSize, prefilterLevelCount, plainImage, plainImageView);
    createCubeImage(irradianceSize, 1, irradianceImage, irradianceImageView);
    createCubeImage(defaultSize, prefilterLevelCount, prefilterImage, prefilterImageView);
    createRenderPass();
    createDescriptors();

    Offscreen::renderPass = renderPass;
    Offscreen::format = format;
    PipelineHelper::descriptorSetLayout = descriptorSetLayout;
    PipelineHelper::renderPass = renderPass;

    VkPushConstantRange plainRange{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(plainPushConstants)};
    VkPushConstantRange irradianceRange{
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(irradiancePushConstants)};
    VkPushConstantRange prefilterRange{
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(prefilterPushConstants)};

    createEnvironmentImage(defaultSize, plainRange, "plain", texture.getImageView(), Target::plain);
    createEnvironmentImage(irradianceSize, irradianceRange, "irradiance", plainImageView, Target::irradiance);
    createEnvironmentImage(defaultSize, prefilterRange, "prefilter", plainImageView, Target::prefilter);
}

VkImageView EnvironmentImages::getPlainImageView() const
{
    return plainImageView;
}

VkImageView EnvironmentImages::getIrradianceImageView() const
{
    return irradianceImageView;
}

VkImageView EnvironmentImages::getPrefilterImageView() const
{
    return prefilterImageView;
}

void EnvironmentImages::loadModel()
{
    fw::Model model;
    CHECK(model.loadModel(assetsFolder + "cube.3ds"));

    fw::Model::Meshes meshes = model.getMeshes();
    CHECK(meshes.size() == 1);

    const fw::Mesh& mesh = meshes[0];

    bool success = vertexBuffer.createForDevice<glm::vec3>(mesh.positions, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        && indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    CHECK(success);

    numIndices = fw::ui32size(mesh.indices);
}

void EnvironmentImages::createCubeImage(uint32_t size, uint32_t mipLevels, fw::Image& image, VkImageView& imageView)
{
    VkImageCreateFlags flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    CHECK(image.create(size, size, format, flags, imageUsage, 6, mipLevels, VK_SAMPLE_COUNT_1_BIT));

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = format;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = 6;
    viewInfo.image = image.getHandle();
    VK_CHECK(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView));
}

void EnvironmentImages::createRenderPass()
{
    VkAttachmentDescription colorAttachment = fw::RenderPass::getColorAttachment();
    colorAttachment.format = format;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass));
}

void EnvironmentImages::createDescriptors()
{
    // Pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 2;

    VK_CHECK(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool));

    // Layout
    VkDescriptorSetLayoutBinding setLayoutBinding{};
    setLayoutBinding.binding = 0;
    setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    setLayoutBinding.descriptorCount = 1;
    setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    setLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = &setLayoutBinding;
    layoutInfo.bindingCount = 1;

    VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout));

    // Descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    VK_CHECK(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet));
}

void EnvironmentImages::createEnvironmentImage(int32_t textureSize,
                                               VkPushConstantRange range,
                                               const std::string& shader,
                                               VkImageView input,
                                               Target target)
{
    std::string vertexShader = shaderFolder + "environment_cube.vert.spv";
    std::string fragmentShader = shaderFolder + shader + ".frag.spv";

    Offscreen offscreen;
    PipelineHelper pipelineHelper;
    offscreen.createFramebuffer(textureSize);
    pipelineHelper.createPipeline(textureSize, range, vertexShader, fragmentShader);

    updateDescriptors(input);
    render(offscreen, pipelineHelper, target);
    changeLayoutToShaderRead(target);
}

void EnvironmentImages::updateDescriptors(VkImageView imageView)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler.getSampler();

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

fw::Image& EnvironmentImages::getImageByTarget(Target target)
{
    if (target == Target::plain)
    {
        return plainImage;
    }
    else if (target == Target::irradiance)
    {
        return irradianceImage;
    }
    else
    {
        return prefilterImage;
    }
}

uint32_t EnvironmentImages::getLevelCountByTarget(Target target)
{
    return target == Target::irradiance ? 1 : prefilterLevelCount;
}

void EnvironmentImages::render(Offscreen& offscreen, PipelineHelper& pipelineHelper, Target target)
{
    fw::Image& targetImage = getImageByTarget(target);
    uint32_t levelCount = getLevelCountByTarget(target);

    VkClearValue clearValues;
    clearValues.color = {0.0f, 0.0f, 0.2f, 0.0f};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = offscreen.getFramebuffer();
    renderPassBeginInfo.renderArea.extent.width = offscreen.getSize();
    renderPassBeginInfo.renderArea.extent.height = offscreen.getSize();
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValues;

    VkCommandBuffer cmd = fw::Command::beginSingleTimeCommands();

    VkViewport viewport{};
    viewport.width = static_cast<float>(offscreen.getSize());
    viewport.height = static_cast<float>(offscreen.getSize());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkImageSubresourceRange cubeSubresourceRange{};
    cubeSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubeSubresourceRange.baseMipLevel = 0;
    cubeSubresourceRange.levelCount = levelCount;
    cubeSubresourceRange.layerCount = 6;

    {
        // Change target cube map layout
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.image = targetImage.getHandle();
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.subresourceRange = cubeSubresourceRange;
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    for (uint32_t level = 0; level < levelCount; ++level)
    {
        viewport.width = static_cast<float>(offscreen.getSize() * std::pow(0.5f, level));
        viewport.height = static_cast<float>(offscreen.getSize() * std::pow(0.5f, level));

        for (uint32_t face = 0; face < 6; ++face)
        {
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkPipelineLayout layout = pipelineHelper.getPipelineLayout();
            VkShaderStageFlags stage = pipelineHelper.getPushConstantRange().stageFlags;
            uint32_t size = pipelineHelper.getPushConstantRange().size;
            void* data;

            switch (target)
            {
            case Target::plain:
                plainPushConstants.mvp
                    = glm::perspective(glm::pi<float>() / 2.0f, 1.0f, 0.1f, 10.0f) * viewMatrices[face];
                data = &plainPushConstants;
                break;
            case Target::irradiance:
                irradiancePushConstants.mvp
                    = glm::perspective(glm::pi<float>() / 2.0f, 1.0f, 0.1f, 10.0f) * invertViewMatrices[face];
                irradiancePushConstants.deltaPhi = (2.0f * glm::pi<float>()) / 180.0f;
                irradiancePushConstants.deltaTheta = (0.5f * glm::pi<float>()) / 64.0f;
                data = &irradiancePushConstants;
                break;
            case Target::prefilter:
                prefilterPushConstants.mvp
                    = glm::perspective(glm::pi<float>() / 2.0f, 1.0f, 0.1f, 10.0f) * invertViewMatrices[face];
                prefilterPushConstants.numSamples = 32u;
                prefilterPushConstants.roughness = static_cast<float>(level) / static_cast<float>(levelCount - 1);
                data = &prefilterPushConstants;
                break;
            }

            VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            vkCmdPushConstants(cmd, layout, stage, 0, size, data);
            vkCmdBindPipeline(cmd, bindPoint, pipelineHelper.getPipeline());
            vkCmdBindDescriptorSets(
                cmd, bindPoint, pipelineHelper.getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
            VkBuffer vb = vertexBuffer.getBuffer();
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd, 0, 1, &vb, offsets);
            vkCmdBindIndexBuffer(cmd, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, numIndices, 1, 0, 0, 0);
            vkCmdEndRenderPass(cmd);

            // Transfer rendered output
            {
                VkImageSubresourceRange subresourceRange{};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.levelCount = 1;
                subresourceRange.layerCount = 1;

                VkImageMemoryBarrier imageMemoryBarrier{};
                imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                imageMemoryBarrier.image = offscreen.getImageHandle();
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.subresourceRange = subresourceRange;
                VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                vkCmdPipelineBarrier(
                    cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            }

            // Copy rendered images
            VkImageCopy copyRegion{};
            copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.srcSubresource.baseArrayLayer = 0;
            copyRegion.srcSubresource.mipLevel = 0;
            copyRegion.srcSubresource.layerCount = 1;
            copyRegion.srcOffset = {0, 0, 0};
            copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.dstSubresource.baseArrayLayer = face;
            copyRegion.dstSubresource.layerCount = 1;
            copyRegion.dstSubresource.mipLevel = level;
            copyRegion.dstOffset = {0, 0, 0};
            copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
            copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
            copyRegion.extent.depth = 1;

            VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            vkCmdCopyImage(
                cmd, offscreen.getImageHandle(), srcLayout, targetImage.getHandle(), dstLayout, 1, &copyRegion);
        }
    }

    fw::Command::endSingleTimeCommands(cmd);
}

void EnvironmentImages::changeLayoutToShaderRead(Target target)
{
    fw::Image& targetImage = getImageByTarget(target);
    uint32_t levelCount = getLevelCountByTarget(target);

    VkCommandBuffer cmd = fw::Command::beginSingleTimeCommands();

    VkImageSubresourceRange cubeSubresourceRange{};
    cubeSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubeSubresourceRange.baseMipLevel = 0;
    cubeSubresourceRange.levelCount = levelCount;
    cubeSubresourceRange.layerCount = 6;

    // Change cube map layout back
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.image = targetImage.getHandle();
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.subresourceRange = cubeSubresourceRange;
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    fw::Command::endSingleTimeCommands(cmd);
}
