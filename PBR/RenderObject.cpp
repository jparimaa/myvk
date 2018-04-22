#include "RenderObject.h"

#include "../Framework/RenderPass.h"
#include "../Framework/Context.h"
#include "../Framework/Common.h"
#include "../Framework/Pipeline.h"
#include "../Framework/Model.h"

RenderObject::~RenderObject()
{
    vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
}

bool RenderObject::initialize(VkRenderPass pass, VkDescriptorPool pool, VkSampler textureSampler)
{
    textures =
    {
        {aiTextureType_DIFFUSE, {fw::Texture(), 1}},
        {aiTextureType_EMISSIVE, {fw::Texture(), 2}},
        {aiTextureType_NORMALS, {fw::Texture(), 3}},
        {aiTextureType_LIGHTMAP, {fw::Texture(), 4}}
    };

    renderPass = pass;
    descriptorPool = pool;
    sampler = textureSampler;
    logicalDevice = fw::Context::getLogicalDevice();

    bool success =
        createDescriptorSetLayout() &&
        createPipeline() &&
        createRenderObject();

    return success;
}

void RenderObject::update(const fw::Camera& camera)
{
    TransformMatrices matrices;
    transformation.setPosition(0.0f, 0.0f, -3.0f);
    transformation.setRotation(1.57f, 0.7f, 0.0f);
    matrices.world = transformation.getWorldMatrix();
    matrices.view = camera.getViewMatrix();
    matrices.proj = camera.getProjectionMatrix();
    transformationBuffer.setData(sizeof(matrices), &matrices);
}

void RenderObject::render(VkCommandBuffer cb)
{
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[] = {0};
    VkBuffer vb = vertexBuffer.getBuffer();
    vkCmdBindVertexBuffers(cb, 0, 1, &vb, offsets);
    vkCmdBindIndexBuffer(cb, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdDrawIndexed(cb, numIndices, 1, 0, 0, 0);
}

bool RenderObject::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;  // Optional

    std::vector<VkDescriptorSetLayoutBinding> bindings{ uboLayoutBinding };

    for (const auto& kv : textures)
    {
        VkDescriptorSetLayoutBinding textureBinding{};
        textureBinding.binding = kv.second.binding;
        textureBinding.descriptorCount = 1;
        textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureBinding.pImmutableSamplers = nullptr;
        bindings.push_back(textureBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = fw::ui32size(bindings);
    layoutInfo.pBindings = bindings.data();

    if (VkResult r = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a render object descriptor set layout", &r);
        return false;
    }
    return true;
}

bool RenderObject::createPipeline()
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
        fw::Pipeline::getShaderStageInfos("pbr_vert.spv", "pbr_frag.spv");

    if (shaderStages.empty()) {
        return false;
    }

    fw::Cleaner cleaner([&shaderStages, this]() {
            for (const auto& info : shaderStages) {
                vkDestroyShaderModule(logicalDevice, info.module, nullptr);
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
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = fw::Pipeline::getColorBlendState();
    VkPipelineColorBlendStateCreateInfo colorBlendState = fw::Pipeline::getColorBlendInfo(&colorBlendAttachmentState);
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = fw::Pipeline::getPipelineLayoutInfo(&descriptorSetLayout);

    if (VkResult r = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a render object pipeline layout", &r);
        return false;
    }

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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (VkResult r = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        r != VK_SUCCESS) {
        fw::printError("Failed to create a render object pipeline", &r);
        return false;
    }

    return true;
}

bool RenderObject::createRenderObject()
{
    VkMemoryPropertyFlags uboProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    bool success = transformationBuffer.create(transformMatricesSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboProperties);

    fw::Model model;
    if (!model.loadModel(assetsFolder + "DamagedHelmet.gltf")) {
        return false;
    }

    fw::Model::Meshes meshes = model.getMeshes();
    if (meshes.size() != 1) {
        fw::printError("Expected that render object has only one mesh");
        return false;
    }

    const fw::Mesh& mesh = meshes[0];
    numIndices = mesh.indices.size();

    success =
        vertexBuffer.createForDevice<fw::Mesh::Vertex>(mesh.getVertices(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)  &&
        indexBuffer.createForDevice<uint32_t>(mesh.indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    if (!allocateDescriptorSet()) {
        return false;
    }

    for (const auto& kv : textures) {
        std::string indexStr = mesh.materials.at(kv.first).front();
        indexStr = indexStr.substr(1);
        int index = std::stoi(indexStr);
        const std::vector<unsigned char>& textureData = model.getTextureData(index);
        TextureBinding& t = textures[kv.first];
        t.texture.load(textureData.data(), textureData.size());
    }

    updateDescriptorSet();

    return success;
}

bool RenderObject::allocateDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (VkResult r = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet);
        r != VK_SUCCESS) {
        fw::printError("Failed to allocate render object descriptor set", &r);
        return false;
    }
    return true;
}

void RenderObject::updateDescriptorSet()
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = transformationBuffer.getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = transformMatricesSize;

    VkWriteDescriptorSet bufferWrite{};
    bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferWrite.pNext = nullptr;
    bufferWrite.dstSet = descriptorSet;
    bufferWrite.dstBinding = 0;
    bufferWrite.dstArrayElement = 0;
    bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferWrite.descriptorCount = 1;
    bufferWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(logicalDevice, 1, &bufferWrite, 0, nullptr);

    for (const auto& kv : textures)
    {
        VkImageView imageView = kv.second.texture.getImageView();
        uint32_t binding = kv.second.binding;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageWrite.pNext = nullptr;
        imageWrite.dstSet = descriptorSet;
        imageWrite.dstBinding = binding;
        imageWrite.dstArrayElement = 0;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageWrite.descriptorCount = 1;
        imageWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(logicalDevice, 1, &imageWrite, 0, nullptr);
    }

    // Todo: for some reason could not create an array of
    // VkWriteDescriptorSet and update all the bindings with a single
    // vkUpdateDescriptorSets command
}
