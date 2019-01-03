#include "DebugDraw.h"

#include "fw/Context.h"
#include "fw/API.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <algorithm>

void DebugDraw::initialize(const Buffers& buffers)
{
    m_buffers = buffers;
}

void DebugDraw::writeImages(const Matrices& matrices)
{
    writeLights(matrices);
    writeTiles();
}

void DebugDraw::writeLights(const Matrices& matrices)
{
    void* mappedMemory = NULL;
    VkDevice logicalDevice = fw::Context::getLogicalDevice();
    vkMapMemory(logicalDevice, m_buffers.lightBuffer->getMemory(), 0, c_lightBufferSize, 0, &mappedMemory);
    Light* lightMemory = (Light*)mappedMemory;

    VkExtent2D extent = fw::API::getSwapChainExtent();
    int numComponents = 3;
    std::vector<uint8_t> image(extent.width * extent.height * numComponents, 0);

    for (int i = 0; i < c_numLights; ++i)
    {
        glm::vec4 pos = lightMemory[i].position;
        pos = matrices.proj * matrices.view * pos;
        glm::vec2 ndc{pos.x / pos.w, pos.y / pos.w};
        glm::vec2 uv2{(0.5 * ndc.x) + (0.5), (0.5 * ndc.y) + (0.5)};
        if (uv2.x > 1.0f || uv2.x < 0.0f || uv2.y > 1.0f || uv2.y < 0.0f)
        {
            continue;
        }

        int x = static_cast<int>(uv2.x * static_cast<float>(extent.width));
        int y = static_cast<int>(uv2.y * static_cast<float>(extent.height));
        int index = numComponents * (y * extent.width + x);
        image[index + 0] = 255;
        image[index + 1] = 255;
        image[index + 2] = 255;
    }

    std::string fileName = "lights.png";

    stbi_write_png(fileName.c_str(), extent.width, extent.height, numComponents, image.data(), 0);
    std::cout << "Wrote file " << fileName << "\n";

    vkUnmapMemory(logicalDevice, m_buffers.lightBuffer->getMemory());
}

void DebugDraw::writeTiles()
{
    void* mappedMemory = NULL;
    VkDevice logicalDevice = fw::Context::getLogicalDevice();
    vkMapMemory(logicalDevice, m_buffers.tileBuffer->getMemory(), 0, c_tileBufferSize, 0, &mappedMemory);
    uint32_t* tileMemory = (uint32_t*)mappedMemory;

    int numCells = c_gridSize * c_gridSize;
    int numComponents = 3;
    std::vector<uint8_t> tileLights(numCells * numComponents, 0);

    int colorMultiplier = 64;
    for (int depth = 0; depth < c_gridDepth; ++depth)
    {
        for (int i = 0; i < numCells; ++i)
        {
            int numLightsPerTile = 0;
            for (int j = 0; j < c_maxLightsPerTile; ++j)
            {
                int index = (depth * numCells * c_maxLightsPerTile) + (i * c_maxLightsPerTile) + j;
                if (tileMemory[index] != 0)
                {
                    ++numLightsPerTile;
                }
                else
                {
                    break;
                }
            }
            int tileLightsIndex = i * numComponents;
            tileLights[tileLightsIndex] += std::clamp(numLightsPerTile * colorMultiplier, 0, 255);
        }
    }

    std::string fileName = "heatmap.png";
    VkExtent2D extent = fw::API::getSwapChainExtent();
    std::vector<uint8_t> tileImage(extent.width * extent.height * numComponents);
    stbir_resize_uint8(tileLights.data(), c_gridSize, c_gridSize, 0, tileImage.data(), extent.width, extent.height, 0, numComponents);
    stbi_write_png(fileName.c_str(), extent.width, extent.height, numComponents, tileImage.data(), 0);
    std::cout << "Wrote file " << fileName << "\n";

    vkUnmapMemory(logicalDevice, m_buffers.tileBuffer->getMemory());
}
