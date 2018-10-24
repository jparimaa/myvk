#include "LightShaftCreator.h"

LightShaftCreator::~LightShaftCreator()
{
}

bool LightShaftCreator::initialize(uint32_t width, uint32_t height, VkDescriptorSetLayout matrixDescriptorSetLayout)
{
    return m_preLightShaft.initialize(width, height, matrixDescriptorSetLayout);
}

void LightShaftCreator::update(const fw::Camera& camera)
{
    m_preLightShaft.update(camera);
}

void LightShaftCreator::writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects)
{
    m_preLightShaft.writeRenderCommands(cb, renderObjects);
}
