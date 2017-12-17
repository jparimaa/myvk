#include "Pipeline.h"
#include "Common.h"
#include "Model.h"

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

VkPipelineInputAssemblyStateCreateInfo Pipeline::getDefaultInputAssemblyInfo() {
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyInfo;
} 

} // namespace fw
