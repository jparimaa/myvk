#pragma once

#include <glm/glm.hpp>

#include <string>
#include <array>

struct Matrices
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Light
{
    glm::vec4 position;
    glm::vec4 color;
};

const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;

const std::size_t c_transformMatricesSize = sizeof(Matrices);
const int c_numLights = 2048;
const int c_bufferSize = sizeof(Light) * c_numLights;
const int c_workgroupSize = 16;
const int c_gridDepthSplitCount = 4;
const int c_numGridDepthValues = c_gridDepthSplitCount - 1;
const std::array<float, c_numGridDepthValues> c_gridDepthValues{
    5.0f,
    20.0f,
    50.0f};