#pragma once

#include <glm/glm.hpp>
#include <assimp/material.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace fw
{

class Model
{
public:
	struct Mesh
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec2> uvs;
		std::vector<uint32_t> indices;

		using Materials = std::unordered_map<aiTextureType, std::vector<std::string>>;
		Materials materials;

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec2 uv;
        };
        
        using Vertices = std::vector<Vertex>;
        Vertices getVertices() const;
	};

	using Meshes = std::vector<Mesh>;
    
	Model() {};
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;
	
	bool loadModel(const std::string& file);
	const Meshes& getMeshes() const;

private:
	Meshes meshes;
};

} // namespace fw
