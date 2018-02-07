#pragma once

#include <glm/glm.hpp>
#include <assimp/material.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace fw
{

class Mesh
{
public:    
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 uv;
    };

    using Vertices = std::vector<Vertex>;
    using Materials = std::unordered_map<aiTextureType, std::vector<std::string>>;
    
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec2> uvs;
    std::vector<uint32_t> indices;    
    Materials materials;        
    
	Mesh() {};
    Vertices getVertices() const;
    std::string getFirstTextureOfType(aiTextureType type) const;
};

} // namespace fw
