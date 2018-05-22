#include "Sampler.h"
#include "Context.h"
#include "Common.h"

namespace fw
{

Sampler::Sampler()
{
}

Sampler::~Sampler()
{
    vkDestroySampler(Context::getLogicalDevice(), m_sampler, nullptr);
}

bool Sampler::create(VkCompareOp compareOp)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = compareOp;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 512.0f;

    if (VkResult r = vkCreateSampler(Context::getLogicalDevice(), &samplerInfo, nullptr, &m_sampler);
        r != VK_SUCCESS) {
        printError("Failed to create a sampler");
        return false;
    }
    return true;
}

VkSampler Sampler::getSampler() const
{
    return m_sampler;
}

} // namespace fw
