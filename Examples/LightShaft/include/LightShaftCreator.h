#pragma once

#include "LightShaftCommon.h"
#include "PreLightShaft.h"
#include "PostLightShaft.h"
#include "fw/Camera.h"

#include <vulkan/vulkan.h>

#include <vector>

class LightShaftCreator
{
public:
    LightShaftCreator(){};
    ~LightShaftCreator();
    bool initialize(uint32_t width, uint32_t height, VkDescriptorSetLayout matrixDescriptorSetLayout);
    void update(const fw::Camera& camera);
    void writeRenderCommands(VkCommandBuffer cb, const std::vector<RenderObject>& renderObjects);

private:
    PreLightShaft m_preLightShaft;
    PostLightShaft m_postLightShaft;
};
