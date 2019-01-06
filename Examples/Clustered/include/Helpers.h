#pragma once

#include <fw/Buffer.h>

#include <glm/glm.hpp>

#include <string>
#include <array>

struct Matrices
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 inverseProj;
};

struct Light
{
    glm::vec4 position;
    glm::vec4 color;
};

struct SceneInfo
{
    float ncp;
    float fcp;
    uint32_t lightCount;
    uint32_t maxLightsPerTile;
};

struct Buffers
{
    fw::Buffer* matrixBuffer;
    fw::Buffer* sceneBuffer;
    fw::Buffer* lightBuffer;
    fw::Buffer* tileBuffer;
    fw::Buffer* numLightsPerTileBuffer;
};

const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;

const std::size_t c_transformMatricesSize = sizeof(Matrices);
const std::size_t c_sceneInfoSize = sizeof(SceneInfo);
const int c_numLights = 64;
const int c_maxLightsPerTile = 16;
const int c_gridSize = 32;
const int c_gridDepth = 4;
const int c_cellCount = c_gridSize * c_gridSize * c_gridDepth;
const int c_lightBufferSize = sizeof(Light) * c_numLights;
const int c_tileBufferSize = c_cellCount * c_maxLightsPerTile * sizeof(uint32_t);
const int c_numLightsPerTileBufferSize = c_cellCount * sizeof(uint32_t);
