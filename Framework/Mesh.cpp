#include "Mesh.h"

#include <utility>

namespace fw
{

Mesh::Vertices Mesh::getVertices() const
{
    Vertices vertices;
    for (unsigned int i = 0; i < positions.size(); ++i) {
        Vertex v;
        v.position = positions[i];
        v.normal = normals[i];
        v.tangent = tangents[i];
        v.uv = uvs[i];
        vertices.push_back(std::move(v));
    }
    return vertices;
}

} // namespace fw
