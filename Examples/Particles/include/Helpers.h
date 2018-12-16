#pragma once

#include <glm/glm.hpp>

#include <string>

struct Matrices
{
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Particle
{
    glm::vec4 position;
    glm::vec4 direction;
};

const std::string c_assetsFolder = ASSETS_PATH;
const std::string c_shaderFolder = SHADER_PATH;

const std::size_t c_transformMatricesSize = sizeof(Matrices);
const int c_numParticles = 16000;
const int c_bufferSize = sizeof(Particle) * c_numParticles;
const int c_workgroupSize = 32;
const float c_initialSpeed = 100.0f;