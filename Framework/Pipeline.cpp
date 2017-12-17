#include "Pipeline.h"
#include "Common.h"
#include "Model.h"
#include "API.h"

namespace fw {

std::vector<VkPipelineShaderStageCreateInfo> Pipeline::getDefaultShaderStageInfos(
    const std::string& vertexShaderFilename,
    const std::string& fragmentShaderFilename) {
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

VkVertexInputBindingDescription Pipeline::getDefaultVertexDescription() {
    VkVertexInputBindingDescription vertexDescription = {};
    vertexDescription.binding = 0;
    vertexDescription.stride = sizeof(Model::Vertex);
    vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return vertexDescription;
}

std::vector<VkVertexInputAttributeDescription> Pipeline::getDefaultAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Model::Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Model::Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Model::Vertex, texCoord);

    return attributeDescriptions;
}

VkPipelineVertexInputStateCreateInfo Pipeline::getDefaultVertexInputInfo(
    const VkVertexInputBindingDescription* vertexDescription,
    const std::vector<VkVertexInputAttributeDescription>* attributeDescriptions) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = vertexDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions->data();
    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::getDefaultInputAssemblyInfo() {
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyInfo;
} 

VkViewport Pipeline::getDefaultViewport() {
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

VkRect2D Pipeline::getDefaultScissorRect() {
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = API::getSwapChainExtent();
    return scissor;
}

VkPipelineViewportStateCreateInfo Pipeline::getDefaultViewportState(const VkViewport* viewport, const VkRect2D* scissor) {
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = scissor;
    return viewportState;
}    

VkPipelineRasterizationStateCreateInfo Pipeline::getDefaultRasterizationInfo() {
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.lineWidth = 1.0f;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor = 0.0f;
    rasterizationInfo.depthBiasClamp = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor = 0.0f;
    return rasterizationInfo;
}

VkPipelineMultisampleStateCreateInfo Pipeline::getDefaultMultisampleInfo() {
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.minSampleShading = 1.0f;           // Optional
    multisampleInfo.pSampleMask = nullptr;             // Optional
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    return multisampleInfo;
}

VkPipelineDepthStencilStateCreateInfo Pipeline::getDefaultDepthStencilInfo() {
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    return depthStencilInfo;
}
    
VkPipelineColorBlendAttachmentState Pipeline::getDefaultColorBlendState() {
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

VkPipelineColorBlendStateCreateInfo Pipeline::getDefaultColorBlendInfo(const VkPipelineColorBlendAttachmentState* colorBlendState) {
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

} // namespace fw
