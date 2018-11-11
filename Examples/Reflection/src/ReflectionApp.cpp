#include "ReflectionApp.h"
#include "fw/API.h"
#include "fw/Command.h"
#include "fw/Common.h"
#include "fw/Constants.h"
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
const std::size_t c_transformMatricesSize = sizeof(glm::mat4x4) * 3;
const uint32_t c_gbufferTextureCount = 3;
// + 1 = final composite
const uint32_t c_colorAttachmentCount = 3 + 1;
// + 2 = Depth & final composite
const uint32_t c_totalAttachmentCount = c_gbufferTextureCount + 2;

const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;

} // unnamed

ReflectionApp::Subpass::~Subpass()
{
    VkDevice logicalDevice = fw::Context::getLogicalDevice();
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
}

ReflectionApp::FramebufferAttachment::~FramebufferAttachment()
{
    VkDevice logicalDevice = fw::Context::getLogicalDevice();
    vkDestroyImageView(logicalDevice, imageView, nullptr);
}

ReflectionApp::~ReflectionApp()
{
    for (VkFramebuffer fb : m_framebuffers)
    {
        vkDestroyFramebuffer(m_logicalDevice, fb, nullptr);
    }
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);
}

bool ReflectionApp::initialize()
{
    m_logicalDevice = fw::Context::getLogicalDevice();

    CHECK(fw::API::initializeSwapChain());
    createGBufferAttachments();
    createRenderPass();
    createFramebuffers();
    createDescriptorSetLayouts();
    createGBufferPipeline();
    createCompositePipeline();
    CHECK(m_sampler.create(VK_COMPARE_OP_ALWAYS));
    createDescriptorPool();
    createRenderObjects();
    createAndUpdateCompositeDescriptorSet();
    createCommandBuffers();

    m_cameraController.setCamera(&m_camera);
    glm::vec3 initPos(0.0f, 10.0f, 40.0f);
    m_cameraController.setResetMode(initPos, glm::vec3(), GLFW_KEY_R);
    m_camera.setPosition(initPos);

    m_droid.matrices.proj = m_camera.getProjectionMatrix();
    m_cube.transformation.setScale(45.0f);
    m_cube.transformation.setPosition(0.0f, -45.0f, 0.0f);
    m_cube.matrices.proj = m_camera.getProjectionMatrix();

    return true;
}

void ReflectionApp::update()
{
    m_droid.transformation.rotateUp(fw::API::getTimeDelta() * glm::radians(45.0f));
    m_droid.matrices.world = m_droid.transformation.getWorldMatrix();
    m_cube.matrices.world = m_cube.transformation.getWorldMatrix();

    m_cameraController.update();
    m_droid.matrices.view = m_camera.getViewMatrix();
    m_cube.matrices.view = m_camera.getViewMatrix();

    m_droid.uniformBuffer.setData(sizeof(m_droid.matrices), &m_droid.matrices);
    m_cube.uniformBuffer.setData(sizeof(m_cube.matrices), &m_cube.matrices);
}

void ReflectionApp::createGBufferAttachments()
{
    auto createAttachment = [this](VkFormat format, FramebufferAttachment& attachment) {
        VkExtent2D extent = fw::API::getSwapChainExtent();
        VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        attachment.image.create(extent.width, extent.height, format, 0, usage);
        attachment.image.createView(format, VK_IMAGE_ASPECT_COLOR_BIT, &attachment.imageView);
        attachment.format = format;
    };

    m_framebufferAttachments.resize(c_gbufferTextureCount);

    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, m_framebufferAttachments[0]); // Positions
    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, m_framebufferAttachments[1]); // Normals
    createAttachment(VK_FORMAT_R8G8B8A8_UNORM, m_framebufferAttachments[2]); // Albedo
}

void ReflectionApp::createRenderPass()
{
    std::array<VkSubpassDescription, 2> subpassDescriptions{};

    // First subpass for creating the G-Buffer
    VkAttachmentReference colorReferences[c_gbufferTextureCount];
    colorReferences[0] = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    colorReferences[1] = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    colorReferences[2] = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthReference = {4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].colorAttachmentCount = c_gbufferTextureCount;
    subpassDescriptions[0].pColorAttachments = colorReferences;
    subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

    // Second subpass for composition using G-Buffer outputs
    VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference inputReferences[c_gbufferTextureCount];
    inputReferences[0] = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    inputReferences[1] = {2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    inputReferences[2] = {3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[1].colorAttachmentCount = 1;
    subpassDescriptions[1].pColorAttachments = &colorReference;
    subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
    subpassDescriptions[1].inputAttachmentCount = c_gbufferTextureCount;
    subpassDescriptions[1].pInputAttachments = inputReferences;

    // Subpass dependencies
    std::array<VkSubpassDependency, 3> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = 1;
    dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Attachments
    VkAttachmentDescription defaultAttachment{};
    defaultAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    defaultAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    defaultAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    defaultAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    defaultAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    defaultAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    defaultAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    defaultAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentDescription> attachments(c_totalAttachmentCount, defaultAttachment);

    // Output attachment
    attachments[0].format = fw::API::getSwapChainImageFormat();
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Position
    attachments[1].format = m_framebufferAttachments[0].format;
    // Normals
    attachments[2].format = m_framebufferAttachments[1].format;
    // Albedo
    attachments[3].format = m_framebufferAttachments[2].format;
    // Depth attachment
    attachments[4].format = fw::Constants::depthFormat;
    attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = fw::ui32size(attachments);
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = fw::ui32size(subpassDescriptions);
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = fw::ui32size(dependencies);
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
}

void ReflectionApp::createFramebuffers()
{
    VkImageView attachments[c_totalAttachmentCount];

    VkExtent2D extent = fw::API::getSwapChainExtent();

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = m_renderPass;
    framebufferCreateInfo.attachmentCount = c_totalAttachmentCount;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = extent.width;
    framebufferCreateInfo.height = extent.height;
    framebufferCreateInfo.layers = 1;

    m_framebuffers.resize(fw::API::getSwapChainImageCount());
    const std::vector<VkImageView>& swapChainImageViews = fw::API::getSwapChainImageViews();
    const VkImageView& depthImageView = fw::API::getSwapChainDepthImageView();

    for (unsigned int i = 0; i < m_framebuffers.size(); ++i)
    {
        attachments[0] = swapChainImageViews[i];
        attachments[1] = m_framebufferAttachments[0].imageView;
        attachments[2] = m_framebufferAttachments[1].imageView;
        attachments[3] = m_framebufferAttachments[2].imageView;
        attachments[4] = depthImageView;
        VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferCreateInfo, nullptr, &m_framebuffers[i]));
    }
}

void ReflectionApp::createDescriptorSetLayouts()
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
    createDescriptorSetLayout(gbufferBindings, m_gbuffer.descriptorSetLayout);

    // Composite
    VkDescriptorSetLayoutBinding positionSamplerBinding{};
    positionSamplerBinding.binding = 0;
    positionSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    positionSamplerBinding.descriptorCount = 1;
    positionSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    positionSamplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalSamplerBinding{};
    normalSamplerBinding.binding = 1;
    normalSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    normalSamplerBinding.descriptorCount = 1;
    normalSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalSamplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding albedoSamplerBinding{};
    albedoSamplerBinding.binding = 2;
    albedoSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    albedoSamplerBinding.descriptorCount = 1;
    albedoSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoSamplerBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> compositeBindings{positionSamplerBinding, normalSamplerBinding, albedoSamplerBinding};
    createDescriptorSetLayout(compositeBindings, m_composite.descriptorSetLayout);
}

void ReflectionApp::createGBufferPipeline()
{
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
    VkPipelineLayoutCreateInfo m_pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&m_gbuffer.descriptorSetLayout);
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(c_gbufferTextureCount, colorBlendAttachmentState);
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(nullptr);
    colorBlendState.pAttachments = blendAttachmentStates.data();
    colorBlendState.attachmentCount = c_gbufferTextureCount;

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &m_pipelineLayoutInfo, nullptr, &m_gbuffer.pipelineLayout));

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
    pipelineInfo.layout = m_gbuffer.pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_gbuffer.pipeline));
}

void ReflectionApp::createCompositePipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = fw::Pipeline::getShaderStageInfos(
        c_shaderFolder + "composite.vert.spv", c_shaderFolder + "composite.frag.spv");

    CHECK(!shaderStages.empty());

    fw::Cleaner cleaner([&shaderStages, this]() {
        for (const auto& info : shaderStages)
        {
            vkDestroyShaderModule(m_logicalDevice, info.module, nullptr);
        }
    });

    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = fw::Pipeline::getInputAssemblyState();

    VkViewport viewport = fw::Pipeline::getViewport();
    VkRect2D scissor = fw::Pipeline::getScissorRect();
    VkPipelineViewportStateCreateInfo viewportState = fw::Pipeline::getViewportState(&viewport, &scissor);

    VkPipelineRasterizationStateCreateInfo rasterizationState = fw::Pipeline::getRasterizationState();
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    VkPipelineMultisampleStateCreateInfo multisampleState = fw::Pipeline::getMultisampleState();
    VkPipelineDepthStencilStateCreateInfo depthStencilState = fw::Pipeline::getDepthStencilState();
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendState(&colorBlendAttachmentState);
    VkPipelineLayoutCreateInfo pipelineLayoutInfo
        = fw::Pipeline::getPipelineLayoutInfo(&m_composite.descriptorSetLayout);

    VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_composite.pipelineLayout));

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
    pipelineInfo.layout = m_composite.pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK(
        vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_composite.pipeline));
}

void ReflectionApp::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 16;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 16;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[2].descriptorCount = 16;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = fw::ui32size(poolSizes);
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 16;

    VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void ReflectionApp::createRenderObjects()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto loadModel = [this, uboProperties](const std::string modelName, RenderObject& renderObject, fw::Model::Meshes& meshes) {
        CHECK(renderObject.uniformBuffer.create(c_transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties));
        fw::Model model;
        CHECK(model.loadModel(c_assetsFolder + modelName));
        meshes = model.getMeshes();
        renderObject.objectData.resize(fw::ui32size(meshes));
        createGBufferDescriptorSets(renderObject);
    };

    fw::Model::Meshes droidMeshes;
    fw::Model::Meshes cubeMeshes;
    loadModel("attack_droid.obj", m_droid, droidMeshes);
    loadModel("cube.3ds", m_cube, cubeMeshes);

    uint32_t numMeshes = fw::ui32size(droidMeshes) + fw::ui32size(cubeMeshes);

    bool success = true;
    for (unsigned int i = 0; i < fw::ui32size(droidMeshes); ++i)
    {
        const fw::Mesh& mesh = droidMeshes[i];
        ObjectData& data = m_droid.objectData[i];

        success = success
            && data.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            && data.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        data.numIndices = fw::ui32size(mesh.indices);

        std::string textureFile = c_assetsFolder + mesh.getFirstTextureOfType(aiTextureType::aiTextureType_DIFFUSE);
        data.texture.load(textureFile, VK_FORMAT_R8G8B8A8_UNORM);
        updateGBufferDescriptorSet(m_droid.uniformBuffer.getBuffer(), m_droid.objectData[i].texture.getImageView(), m_droid.objectData[i].descriptorSet);
    }

    for (unsigned int i = 0; i < fw::ui32size(cubeMeshes); ++i)
    {
        const fw::Mesh& mesh = cubeMeshes[i];
        ObjectData& data = m_cube.objectData[i];

        success = success
            && data.vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            && data.indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        data.numIndices = fw::ui32size(mesh.indices);

        std::string textureFile = c_assetsFolder + "checker.png";
        data.texture.load(textureFile, VK_FORMAT_R8G8B8A8_UNORM);
        updateGBufferDescriptorSet(m_cube.uniformBuffer.getBuffer(), m_cube.objectData[i].texture.getImageView(), m_cube.objectData[i].descriptorSet);
    }

    CHECK(success);
}

void ReflectionApp::createGBufferDescriptorSets(RenderObject& renderObject)
{
    uint32_t numSets = fw::ui32size(renderObject.objectData);
    std::vector<VkDescriptorSet> descriptorSets(numSets);

    std::vector<VkDescriptorSetLayout> gbufferLayouts(numSets, m_gbuffer.descriptorSetLayout);
    VkDescriptorSetAllocateInfo gbufferAllocInfo{};
    gbufferAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    gbufferAllocInfo.descriptorPool = m_descriptorPool;
    gbufferAllocInfo.descriptorSetCount = numSets;
    gbufferAllocInfo.pSetLayouts = gbufferLayouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &gbufferAllocInfo, descriptorSets.data()));

    for (uint32_t i = 0; i < numSets; ++i)
    {
        renderObject.objectData[i].descriptorSet = descriptorSets[i];
    }
}

void ReflectionApp::updateGBufferDescriptorSet(VkBuffer buffer, VkImageView imageView, VkDescriptorSet descriptorSet)
{
    VkDescriptorBufferInfo matrixBufferInfo{};
    matrixBufferInfo.buffer = buffer;
    matrixBufferInfo.offset = 0;
    matrixBufferInfo.range = c_transformMatricesSize;

    VkDescriptorImageInfo albedoTextureInfo{};
    albedoTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    albedoTextureInfo.imageView = imageView;
    albedoTextureInfo.sampler = m_sampler.getSampler();

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &matrixBufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &albedoTextureInfo;

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void ReflectionApp::createAndUpdateCompositeDescriptorSet()
{
    std::vector<VkDescriptorSetLayout> compositeLayouts(1, m_composite.descriptorSetLayout);
    VkDescriptorSetAllocateInfo compositeAllocInfo{};
    compositeAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    compositeAllocInfo.descriptorPool = m_descriptorPool;
    compositeAllocInfo.descriptorSetCount = fw::ui32size(compositeLayouts);
    compositeAllocInfo.pSetLayouts = compositeLayouts.data();

    VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &compositeAllocInfo, &m_compositeDescriptorSet));

    uint32_t numAttachments = fw::ui32size(m_framebufferAttachments);
    std::vector<VkWriteDescriptorSet> descriptorWrites(numAttachments);
    std::vector<VkDescriptorImageInfo> imageInfos(numAttachments);

    for (uint32_t i = 0; i < numAttachments; ++i)
    {
        imageInfos[i] = VkDescriptorImageInfo{};
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = m_framebufferAttachments[i].imageView;
        imageInfos[i].sampler = VK_NULL_HANDLE;

        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = m_compositeDescriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pImageInfo = &imageInfos[i];
    }

    vkUpdateDescriptorSets(m_logicalDevice, fw::ui32size(descriptorWrites), descriptorWrites.data(), 0, nullptr);
}

void ReflectionApp::createCommandBuffers()
{
    std::vector<VkCommandBuffer> commandBuffers(fw::API::getSwapChainImageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = fw::API::getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = fw::ui32size(commandBuffers);

    VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, commandBuffers.data()));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    VkClearValue clearValue;
    clearValue.color = {{0.0f, 0.0f, 0.5f, 0.0f}};
    std::vector<VkClearValue> clearValues(c_colorAttachmentCount, clearValue);
    clearValue.depthStencil = {1.0f, 0};
    clearValues.push_back(clearValue);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = fw::API::getSwapChainExtent();
    renderPassInfo.clearValueCount = fw::ui32size(clearValues);
    renderPassInfo.pClearValues = clearValues.data();

    VkDeviceSize offsets[] = {0};

    for (size_t i = 0; i < commandBuffers.size(); ++i)
    {
        VkCommandBuffer cb = commandBuffers[i];

        VK_CHECK(vkBeginCommandBuffer(cb, &beginInfo));

        renderPassInfo.framebuffer = m_framebuffers[i];

        vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // G-Buffer
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbuffer.pipeline);

        for (const ObjectData& data : m_droid.objectData)
        {
            VkBuffer vb = data.vertexBuffer.getBuffer();
            vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
            vkCmdBindIndexBuffer(cb, data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbuffer.pipelineLayout, 0, 1, &data.descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(cb, data.numIndices, 1, 0, 0, 0);
        }

        for (const ObjectData& data : m_cube.objectData)
        {
            VkBuffer vb = data.vertexBuffer.getBuffer();
            vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
            vkCmdBindIndexBuffer(cb, data.indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gbuffer.pipelineLayout, 0, 1, &data.descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(cb, data.numIndices, 1, 0, 0, 0);
        }

        // Composite
        vkCmdNextSubpass(cb, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_composite.pipeline);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_composite.pipelineLayout, 0, 1, &m_compositeDescriptorSet, 0, nullptr);
        vkCmdDraw(cb, 3, 1, 0, 0);

        vkCmdEndRenderPass(cb);

        VK_CHECK(vkEndCommandBuffer(cb));
    }

    fw::API::setCommandBuffers(commandBuffers);
}
