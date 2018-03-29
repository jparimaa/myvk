#include "EnvironmentImages.h"

#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/RenderPass.h"
#include "../Framework/Command.h"
#include "../Framework/Macros.h"
#include "../Framework/Pipeline.h"
#include "../Framework/Model.h"
#include "../Framework/Constants.h"

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace
{

struct PlainPushConstants
{
    glm::mat4 mvp;
};

struct IrradiancePushConstants
{
    glm::mat4 mvp;
    float deltaPhi;
    float deltaTheta;        
};

struct PrefilterPushConstants
{
    glm::mat4 mvp;
    float roughness;
    uint32_t numSamples = 32u;
};

const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
const int32_t defaultSize = 512;
const int32_t irradianceSize = 64;
const uint32_t mipLevelCount = 9;
PlainPushConstants plainPushConstants;
IrradiancePushConstants irradiancePushConstants;
PrefilterPushConstants prefilterPushConstants;

glm::mat4 viewMatrices[] = {
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::right,    fw::Constants::up),
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::left,     fw::Constants::up),
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::down,     fw::Constants::forward),
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::up,     fw::Constants::backward),
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::forward,  fw::Constants::up),
    glm::lookAt(fw::Constants::zeroVec3, fw::Constants::backward, fw::Constants::up)
};

const std::string assetsFolder = "../Assets/";

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

bool EnvironmentImages::initialize(const std::string& filename)
{
    logicalDevice = fw::Context::getLogicalDevice();

    bool success =
        texture.load(filename) &&
        loadModel() &&
        sampler.create(VK_COMPARE_OP_NEVER) &&
        createCubeImage(defaultSize, 1, plainImage, plainImageView) &&
        createCubeImage(irradianceSize, 1, irradianceImage, irradianceImageView) &&
        createCubeImage(defaultSize, mipLevelCount, prefilterImage, prefilterImageView) &&
        createRenderPass() &&
        createDescriptors();

    if (!success) {
        return false;
    }

    Offscreen::renderPass = renderPass;
    Offscreen::format = format;
    PipelineHelper::descriptorSetLayout = descriptorSetLayout;
    PipelineHelper::renderPass = renderPass;

    irradiancePushConstants.deltaPhi = (2.0f * float(M_PI)) / 180.0f;
    irradiancePushConstants.deltaTheta = (0.5f * float(M_PI)) / 64.0f;
    prefilterPushConstants.numSamples = 32u;

    {
        Offscreen offscreen;
        success = success && offscreen.createFramebuffer(defaultSize, 1, 1);
        
        PipelineHelper pipelineHelper;        
        success = success && pipelineHelper.createPipeline(
            defaultSize,
            {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(plainPushConstants)},
            "equirectangular_hdr_vert.spv",
            "equirectangular_hdr_frag.spv");

        if (!success) {
            return false;
        }

        updateDescriptors(texture.getImageView());
        render(offscreen, pipelineHelper, Target::plain);
    }

    {
        Offscreen offscreen;
        success = success && offscreen.createFramebuffer(irradianceSize, 1, 1);

        PipelineHelper pipelineHelper;
        success = success && pipelineHelper.createPipeline(
            irradianceSize,
            {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(irradiancePushConstants)},
            "irradiance_vert.spv",
            "irradiance_frag.spv");

        if (!success) {
            return false;
        }

        updateDescriptors(plainImageView);
        render(offscreen, pipelineHelper, Target::irradiance);
    }
    return true;
}

VkImageView EnvironmentImages::getPlainImageView() const
{
    return plainImageView;
}

VkImageView EnvironmentImages::getIrradianceImageView() const
{
    return irradianceImageView;
}

bool EnvironmentImages::loadModel()
{
    fw::Model model;
    if (!model.loadModel(assetsFolder + "cube.3ds")) {
        return false;
    }

    fw::Model::Meshes meshes = model.getMeshes();
    if (meshes.size() != 1) {
        fw::printError("Expected that skybox has only one mesh");
        return false;
    }

    const fw::Mesh& mesh = meshes[0];
        
    bool success =
        vertexBuffer.createForDevice<glm::vec3>(mesh.positions, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) &&
        indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        
    numIndices = mesh.indices.size();
    return success;
}

bool EnvironmentImages::createCubeImage(uint32_t size, uint32_t mipLevels, fw::Image& image, VkImageView& imageView)
{
    VkImageCreateFlags flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (!image.create(size, size, format, flags, imageUsage, 6, mipLevels)) {
        return false;
    }
 
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = format;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = 6;
    viewInfo.image = image.getHandle();
    if (VkResult r = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);
        r != VK_SUCCESS) {        
        fw::printError("Failed to create image view for HDR equirectangular", &r);
        return false;
    }
    return true;
}

bool EnvironmentImages::createRenderPass()
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
    
    if (VkResult r = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a equirectangular HDR render pass", &r);
        return false;
    }
    return true;
}

bool EnvironmentImages::createDescriptors()
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

    if (VkResult r = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool);
        r != VK_SUCCESS) {
        fw::printError("Failed to create descriptor pool for HDR equirectangular", &r);
        return false;
    }

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
    if (VkResult r = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout);
        r != VK_SUCCESS) {
        fw::printError("Failed to create descriptor set layout for HDR equirectangular", &r);
        return false;
    }

    // Descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
    if (VkResult r = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet);
        r != VK_SUCCESS) {
        fw::printError("Failed to allocate descriptor set HDR equirectangular", &r);
        return false;
    }

    return true;
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

void EnvironmentImages::render(Offscreen& offscreen, PipelineHelper& pipelineHelper, Target target)
{
    fw::Image& targetImage =
        target == Target::plain ? plainImage :
        target == Target::irradiance ? irradianceImage :
        prefilterImage;
    
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

    VkImageSubresourceRange cubeSubresourceRange{};
    cubeSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cubeSubresourceRange.baseMipLevel = 0;
    cubeSubresourceRange.levelCount = 1;
    cubeSubresourceRange.layerCount = 6;
    
    {
        // Change cube map layout
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

    for (uint32_t face = 0; face < 6; ++face) {
        // Draw one layer to offscreen framebuffer
        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkPipelineLayout layout = pipelineHelper.getPipelineLayout();
        VkShaderStageFlags stage = pipelineHelper.getPushConstantRange().stageFlags;
        uint32_t size = pipelineHelper.getPushConstantRange().size;

        void* data;
        glm::mat4 mvp = glm::perspective(glm::pi<float>() / 2.0f, 1.0f, 0.1f, 10.0f) * viewMatrices[face];
        switch (target) {
        case Target::plain:
            plainPushConstants.mvp = mvp;
            data = &plainPushConstants;
            break;
        case Target::irradiance:
            irradiancePushConstants.mvp = mvp;
            data = &irradiancePushConstants;
            break;
        case Target::prefilter:
            prefilterPushConstants.mvp = mvp;
            //prefilterPushConstants.roughness = (float)m / (float)(numMips - 1);
            data = &prefilterPushConstants;
            break;
        }

        vkCmdPushConstants(cmd, layout, stage, 0, size, data);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHelper.getPipeline());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHelper.getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
        VkBuffer vb = vertexBuffer.getBuffer();
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, offsets);
        vkCmdBindIndexBuffer(cmd, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, numIndices, 1, 0, 0, 0);        
        vkCmdEndRenderPass(cmd);

        // Copy rendered images
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
            vkCmdPipelineBarrier(cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        }

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = face;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };
        copyRegion.extent.width = offscreen.getSize();
        copyRegion.extent.height = offscreen.getSize();
        copyRegion.extent.depth = 1;

        VkImageLayout srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        VkImageLayout dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vkCmdCopyImage(cmd, offscreen.getImageHandle(), srcLayout, targetImage.getHandle(), dstLayout, 1, &copyRegion);
    }
/*
    {
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
    }
*/
    fw::Command::endSingleTimeCommands(cmd);    
}
