#include "GBufferPass.h"
#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Context.h"
#include "fw/Macros.h"
#include "fw/Mesh.h"
#include "fw/Model.h"
#include "fw/Pipeline.h"
#include "fw/RenderPass.h"

#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace
{
const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;
const VkFormat c_format = VK_FORMAT_R8G8B8A8_UNORM;
const std::size_t c_transformMatricesSize = sizeof(glm::mat4x4) * 3;
}

GBufferPass::~GBufferPass()
{
    vkDestroyImageView(m_logicalDevice, m_albedo.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_position.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_normal.imageView, nullptr);
    vkDestroyImageView(m_logicalDevice, m_depth.imageView, nullptr);
    vkDestroyFramebuffer(m_logicalDevice, m_framebuffer, nullptr);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

void GBufferPass::initialize(const fw::Camera* camera)
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    createRenderPass();
    createFramebuffer();
    createDescriptorSetLayouts();
    createPipeline();
    CHECK(m_sampler.create(VK_COMPARE_OP_ALWAYS));
    createDescriptorPool();
    createRenderObjects();

    m_camera = camera;
    m_droid.matrices.proj = m_camera->getProjectionMatrix();
    m_cube.transformation.setScale(45.0f);
    m_cube.transformation.setPosition(0.0f, -45.0f, 0.0f);
    m_cube.matrices.proj = m_camera->getProjectionMatrix();
}

void GBufferPass::update()
{
    m_droid.transformation.rotateUp(fw::API::getTimeDelta() * glm::radians(45.0f));
    m_droid.matrices.world = m_droid.transformation.getWorldMatrix();
    m_cube.matrices.world = m_cube.transformation.getWorldMatrix();

    m_droid.matrices.view = m_camera->getViewMatrix();
    m_cube.matrices.view = m_camera->getViewMatrix();

    m_droid.uniformBuffer.setData(sizeof(m_droid.matrices), &m_droid.matrices);
    m_cube.uniformBuffer.setData(sizeof(m_cube.matrices), &m_cube.matrices);
}

void GBufferPass::writeRenderCommands(VkCommandBuffer cb)
{
    std::array<VkClearValue, 4> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[3].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.framebuffer = m_framebuffer;

    VkDeviceSize offsets[] = {0};

    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    renderObject(cb, m_droid, 0.0f);
    renderObject(cb, m_cube, 1.0f);

    vkCmdEndRenderPass(cb);
}

void GBufferPass::renderObject(VkCommandBuffer cb, const RenderObject& object, float reflectivity)
{
    //vkCmdPushConstants(cb, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, c_pushConstantsSize, &reflectivity);

    VkDeviceSize offsets[] = {0};
    for (const ObjectData& data : object.objectData)
    {
        VkBuffer vb = data.vertexBuffer.getBuffer();
        vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
        vkCmdBindIndexBuffer(cb, data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &data.descriptorSet, 0, nullptr);
        vkCmdDrawIndexed(cb, data.numIndices, 1, 0, 0, 0);
    }
}

VkImageView GBufferPass::getAlbedoImageView() const
{
    return m_albedo.imageView;
}

VkImageView GBufferPass::getPositionImageView() const
{
    return m_position.imageView;
}

VkImageView GBufferPass::getNormalImageView() const
{
    return m_normal.imageView;
}

void GBufferPass::createRenderPass()
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkAttachmentReference albedoAttachment{};
    albedoAttachment.attachment = 0;
    albedoAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference positionAttachment{};
    positionAttachment.attachment = 1;
    positionAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference normalAttachment{};
    normalAttachment.attachment = 2;
    normalAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentReference> colorAttachments{albedoAttachment, positionAttachment, normalAttachment};

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = fw::ui32size(colorAttachments);
    subpass.pColorAttachments = colorAttachments.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription colorAttachmentDescription = fw::RenderPass::getColorAttachment();
    colorAttachmentDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentDescription depthAttachment = fw::RenderPass::getDepthAttachment();

    std::vector<VkAttachmentDescription> attachmentDescriptions = {colorAttachmentDescription, colorAttachmentDescription, colorAttachmentDescription, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = fw::ui32size(attachmentDescriptions);
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
}

void GBufferPass::createFramebuffer()
{
    VkExtent2D extent = fw::API::getSwapChainExtent();
    uint32_t width = extent.width;
    uint32_t height = extent.height;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CHECK(m_albedo.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_albedo.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_albedo.imageView));
    CHECK(m_albedo.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    CHECK(m_position.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_position.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_position.imageView));
    CHECK(m_position.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    CHECK(m_normal.image.create(width, height, c_format, 0, usage, 1));
    CHECK(m_normal.image.createView(c_format, VK_IMAGE_ASPECT_COLOR_BIT, &m_normal.imageView));
    CHECK(m_normal.image.transitLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));

    VkFormat depthFormat = fw::Constants::depthFormat;
    VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CHECK(m_depth.image.create(width, height, depthFormat, 0, depthImageUsage, 1));
    CHECK(m_depth.image.createView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &m_depth.imageView));
    CHECK(m_depth.image.transitLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

    std::vector<VkImageView> attachments{m_albedo.imageView, m_position.imageView, m_normal.imageView, m_depth.imageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = fw::ui32size(attachments);
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_framebuffer));
}

void GBufferPass::createDescriptorSetLayouts()
{
    auto createDescriptorSetLayout = [this](const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout& descriptorSetLayout) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = fw::ui32size(bindings);
        layoutInfo.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout));
    };

    // GBuffer
    VkDescriptorSetLayoutBinding matrixBinding{};
    matrixBinding.binding = 0;
    matrixBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matrixBinding.descriptorCount = 1;
    matrixBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    matrixBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> gbufferBindings{matrixBinding, samplerBinding};
    createDescriptorSetLayout(gbufferBindings, m_descriptorSetLayout);
}

void GBufferPass::createPipeline()
{
    std::vector<VkDescriptorSetLayout> layouts{m_descriptorSetLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = fw::ui32size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
        = fw::Pipeline::getShaderStageInfos(c_shaderFolder + "gbuffer.vert.spv", c_shaderFolder + "gbuffer.frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
        for (const auto& info : shaderStages)
        {
            vkDestroyShaderModule(m_logicalDevice, info.module, nullptr);
        }
    });

    VkVertexInputBindingDescription vertexDescription = fw::Pipeline::getVertexDescription();
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = fw::Pipeline::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputState = fw::Pipeline::getVertexInputState(&vertexDescription, &attributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = fw::Pipeline::getInputAssemblyState();

    VkViewport viewport = fw::Pipeline::getViewport();
    VkRect2D scissor = fw::Pipeline::getScissorRect();
    VkPipelineViewportStateCreateInfo viewportState = fw::Pipeline::getViewportState(&viewport, &scissor);

    VkPipelineRasterizationStateCreateInfo rasterizationState = fw::Pipeline::getRasterizationState();
    VkPipelineMultisampleStateCreateInfo multisampleState = fw::Pipeline::getMultisampleState();
    VkPipelineDepthStencilStateCreateInfo depthStencilState = fw::Pipeline::getDepthStencilState();
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates(3, colorBlendAttachmentState);
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(&colorBlendAttachmentState);
    colorBlendState.attachmentCount = 3;
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = fw::ui32size(shaderStages);
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));
}

void GBufferPass::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 3;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void GBufferPass::createRenderObjects()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto loadModel = [this, uboProperties](const std::string modelName, RenderObject& renderObject, std::string texture = "") {
        // Load model
        CHECK(renderObject.uniformBuffer.create(c_transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));
        fw::Model model;
        CHECK(model.loadModel(c_assetsFolder + modelName));
        const fw::Model::Meshes& meshes = model.getMeshes();
        uint32_t numMeshes = fw::ui32size(meshes);
        renderObject.objectData.resize(numMeshes);

        // Allocate desc sets
        std::vector<VkDescriptorSet> descriptorSets(numMeshes);
        std::vector<VkDescriptorSetLayout> gbufferLayouts(numMeshes, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo gbufferAllocInfo{};
        gbufferAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        gbufferAllocInfo.descriptorPool = m_descriptorPool;
        gbufferAllocInfo.descriptorSetCount = numMeshes;
        gbufferAllocInfo.pSetLayouts = gbufferLayouts.data();

        VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &gbufferAllocInfo, descriptorSets.data()));

        for (unsigned int i = 0; i < numMeshes; ++i)
        {
            const fw::Mesh& mesh = meshes[i];
            ObjectData& data = renderObject.objectData[i];

            // Create buffers and textures
            CHECK(data.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
            CHECK(data.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));

            data.numIndices = fw::ui32size(mesh.indices);

            std::string textureFile = c_assetsFolder + (!texture.empty() ? texture : mesh.getFirstTextureOfType(aiTextureType::aiTextureType_DIFFUSE));
            data.texture.load(textureFile, VK_FORMAT_R8G8B8A8_UNORM);

            // Update descriptor set
            data.descriptorSet = descriptorSets[i];

            VkDescriptorBufferInfo matrixBufferInfo{};
            matrixBufferInfo.buffer = renderObject.uniformBuffer.getBuffer();
            matrixBufferInfo.offset = 0;
            matrixBufferInfo.range = c_transformMatricesSize;

            VkDescriptorImageInfo albedoTextureInfo{};
            albedoTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            albedoTextureInfo.imageView = renderObject.objectData[i].texture.getImageView();
            albedoTextureInfo.sampler = m_sampler.getSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = data.descriptorSet;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &matrixBufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = data.descriptorSet;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &albedoTextureInfo;

            vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
        }
    };

    loadModel("attack_droid.obj", m_droid);
    loadModel("cube.3ds", m_cube, "checker.png");
}
