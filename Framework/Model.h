#pragma once

#include <glm/glm.hpp>

namespace fw
{

class Model
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texCoord;
    };
    
    Model() {};
    ~Model() {};
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;
};

} // namespace fw
