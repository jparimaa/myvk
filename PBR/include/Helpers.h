#pragma once

#include <glm/glm.hpp>

#include <string>

struct TransformMatrices
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

const std::size_t transformMatricesSize = sizeof(TransformMatrices);
const std::string assetsFolder = ASSETS_PATH;
const std::string shaderFolder = SHADER_PATH;

