#pragma once

#include <fw/Texture.h>
#include <fw/Buffer.h>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <string>

const std::size_t c_transformMatricesSize = sizeof(glm::mat4x4) * 3;
const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;
const VkFormat c_format = VK_FORMAT_R8G8B8A8_UNORM;

struct RenderObject
{
    fw::Buffer vertexBuffer;
    fw::Buffer indexBuffer;
    uint32_t numIndices;
    fw::Texture texture;
    VkDescriptorSet matrixDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
};

struct MatrixUBO
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};