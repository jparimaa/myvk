#include "Pipeline.h"
#include "Common.h"

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

} // namespace fw
