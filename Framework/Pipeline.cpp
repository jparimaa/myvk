#include "Pipeline.h"
#include "Common.h"
#include "Model.h"
#include "API.h"

namespace fw
{

std::vector<VkPipelineShaderStageCreateInfo> Pipeline::getShaderStageInfos(
    const std::string& vertexShaderFilename,
    const std::string& fragmentShaderFilename)
{
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderFilename);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderFilename);

    auto isNull = [](VkShaderModule shaderModule) {
        return shaderModule == VK_NULL_HANDLE;
    };
    
    if (isNull(vertexShaderModule) || isNull(fragmentShaderModule)) {
        return std::vector<VkPipelineShaderStageCreateInfo>{};
    }

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    return std::vector<VkPipelineShaderStageCreateInfo>{vertexShaderStageInfo, fragmentShaderStageInfo};
}

VkVertexInputBindingDescription Pipeline::getVertexDescription()
{
    VkVertexInputBindingDescription vertexDescription = {};
    vertexDescription.binding = 0;
    vertexDescription.stride = sizeof(Model::Mesh::Vertex);
    vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return vertexDescription;
}

std::vector<VkVertexInputAttributeDescription> Pipeline::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Model::Mesh::Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Model::Mesh::Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Model::Mesh::Vertex, tangent);
    
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Model::Mesh::Vertex, uv);

    return attributeDescriptions;
}

VkPipelineVertexInputStateCreateInfo Pipeline::getVertexInputState(
    const VkVertexInputBindingDescription* vertexDescription,
    const std::vector<VkVertexInputAttributeDescription>* attributeDescriptions)
{
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = vertexDescription;
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size());
    vertexInputState.pVertexAttributeDescriptions = attributeDescriptions->data();
    return vertexInputState;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::getInputAssemblyState()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyState;
} 

VkViewport Pipeline::getViewport()
{
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    VkExtent2D extent = API::getSwapChainExtent();
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkRect2D Pipeline::getScissorRect()
{
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = API::getSwapChainExtent();
    return scissor;
}

VkPipelineViewportStateCreateInfo Pipeline::getViewportState(const VkViewport* viewport, const VkRect2D* scissor)
{
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = scissor;
    return viewportState;
}    

VkPipelineRasterizationStateCreateInfo Pipeline::getRasterizationState()
{
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;
    return rasterizationState;
}

VkPipelineMultisampleStateCreateInfo Pipeline::getMultisampleState()
{
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.minSampleShading = 1.0f;           // Optional
    multisampleState.pSampleMask = nullptr;             // Optional
    multisampleState.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampleState.alphaToOneEnable = VK_FALSE;       // Optional
    return multisampleState;
}

VkPipelineDepthStencilStateCreateInfo Pipeline::getDepthStencilState()
{
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE;
    return depthStencilState;
}
    
VkPipelineColorBlendAttachmentState Pipeline::getColorBlendState()
{
    VkPipelineColorBlendAttachmentState colorBlendState = {};
    colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendState.blendEnable = VK_FALSE;
    colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
    return colorBlendState;
}

VkPipelineColorBlendStateCreateInfo Pipeline::getColorBlendInfo(const VkPipelineColorBlendAttachmentState* colorBlendState)
{
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = colorBlendState;
    colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
    return colorBlendInfo;
}

VkPipelineLayoutCreateInfo Pipeline::getPipelineLayoutInfo(const VkDescriptorSetLayout* setLayout)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = setLayout;
    return pipelineLayoutInfo;
}

} // namespace fw
