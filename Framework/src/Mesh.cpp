#include "Mesh.h"
#include "Common.h"

#include <stdexcept>
#include <utility>

namespace fw
{
Mesh::Vertices Mesh::getVertices() const
{
    Vertices vertices;
    vertices.reserve(positions.size());
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];

        if (!tangents.empty())
        {
            v.tangent = tangents[i];
        }

        if (!uvs.empty())
        {
            v.uv = uvs[i];
        }
        vertices.push_back(std::move(v));
    }
    return vertices;
}

std::string Mesh::getFirstTextureOfType(aiTextureType type) const
{
    std::string ret = "";
    try
    {
        ret = materials.at(type).at(0);
    }
    catch (const std::out_of_range& /*e*/)
    {
        printError("Mesh has no texture for the requested type");
    }
    return ret;
}

} // namespace fw
