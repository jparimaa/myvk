#include "LightShaftCreator.h"

#include "fw/Macros.h"

LightShaftCreator::~LightShaftCreator()
{
}

bool LightShaftCreator::initialize(uint32_t width, uint32_t height, VkDescriptorSetLayout matrixDescriptorSetLayout)
{
    CHECK(m_preLightShaft.initialize(width, height, matrixDescriptorSetLayout));
    CHECK(m_postLightShaft.initialize(m_preLightShaft.getOutputImageView()));
    return true;
}

void LightShaftCreator::update(const fw::Camera& camera)
{
    m_preLightShaft.update(camera);
    m_postLightShaft.update(camera, m_preLightShaft.getLightTransformation());
}

void LightShaftCreator::onGUI()
{
    m_postLightShaft.onGUI();
}

void LightShaftCreator::writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects, VkFramebuffer finalFramebuffer)
{
    m_preLightShaft.writeRenderCommands(cb, renderObjects);
    m_postLightShaft.writeRenderCommands(cb, finalFramebuffer);
}
