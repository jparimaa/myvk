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

struct Buffers
{
    fw::Buffer* matrixBuffer;
    fw::Buffer* lightBuffer;
    fw::Buffer* lightIndexBuffer;
    fw::Buffer* tileBuffer;
};

const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;

const std::size_t c_transformMatricesSize = sizeof(Matrices);
const int c_numLights = 2048;
const int c_gridSize = 32;
const int c_gridDepth = 4;
const int c_cellCount = c_gridSize * c_gridSize * c_gridDepth;
const int c_lightBufferSize = sizeof(Light) * c_numLights;
const int c_lightIndexBufferSize = c_numLights * c_numLights * sizeof(uint32_t);
const int c_tileBufferSize = c_cellCount * sizeof(uint32_t) * 2;
