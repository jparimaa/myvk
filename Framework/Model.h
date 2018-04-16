#pragma once

#include "Mesh.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace fw
{

class Model
{
public:
	using Meshes = std::vector<Mesh>;
    using TextureData = std::vector<unsigned char>;

	Model() {};
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

	bool loadModel(const std::string& file);
	const Meshes& getMeshes() const;
    const TextureData& getTextureData(unsigned int index);

private:
	Meshes meshes;
    std::unordered_map<unsigned int, TextureData> textureDatas;
};

} // namespace fw
