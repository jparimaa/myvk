#pragma once

#include "Mesh.h"

#include <string>
#include <vector>

namespace fw
{

class Model
{
public:
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
