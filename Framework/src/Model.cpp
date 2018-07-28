#include "Model.h"
#include "Common.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <utility>

namespace fw
{
bool Model::loadModel(const std::string& file)
{
    Assimp::Importer importer;
    unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals
        | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace;
    const aiScene* aScene = importer.ReadFile(file, flags);

    if (aScene)
    {
        if (aScene->mNumMeshes == 0)
        {
            printWarning("No mesh found in the model: " + file);
            return false;
        }

        for (unsigned int meshIndex = 0; meshIndex < aScene->mNumMeshes; ++meshIndex)
        {
            const aiMesh* aMesh = aScene->mMeshes[meshIndex];
            Mesh mesh;
            for (unsigned int vertexIndex = 0; vertexIndex < aMesh->mNumVertices; ++vertexIndex)
            {
                mesh.positions.push_back(glm::vec3(
                    aMesh->mVertices[vertexIndex].x, aMesh->mVertices[vertexIndex].y, aMesh->mVertices[vertexIndex].z));

                mesh.normals.push_back(glm::vec3(
                    aMesh->mNormals[vertexIndex].x, aMesh->mNormals[vertexIndex].y, aMesh->mNormals[vertexIndex].z));

                if (aMesh->HasTangentsAndBitangents())
                {
                    mesh.tangents.push_back(glm::vec3(aMesh->mTangents[vertexIndex].x,
                                                      aMesh->mTangents[vertexIndex].y,
                                                      aMesh->mTangents[vertexIndex].z));
                }

                if (aMesh->HasTextureCoords(0))
                {
                    mesh.uvs.push_back(
                        glm::vec2(aMesh->mTextureCoords[0][vertexIndex].x, -aMesh->mTextureCoords[0][vertexIndex].y));
                }
            }

            for (unsigned int faceIndex = 0; faceIndex < aMesh->mNumFaces; ++faceIndex)
            {
                if (aMesh->mFaces[faceIndex].mNumIndices != 3)
                {
                    printWarning("Unable to parse model indices for " + file);
                    return false;
                }
                mesh.indices.push_back(aMesh->mFaces[faceIndex].mIndices[0]);
                mesh.indices.push_back(aMesh->mFaces[faceIndex].mIndices[1]);
                mesh.indices.push_back(aMesh->mFaces[faceIndex].mIndices[2]);
            }

            aiMaterial* aMaterial = aScene->mMaterials[aMesh->mMaterialIndex];
            if (aMaterial)
            {
                for (int typeIndex = 0; typeIndex < aiTextureType_UNKNOWN; ++typeIndex)
                {
                    aiTextureType type = static_cast<aiTextureType>(typeIndex);
                    unsigned int numTextures = aMaterial->GetTextureCount(type);
                    for (unsigned int texIndex = 0; texIndex < numTextures; ++texIndex)
                    {
                        aiString path;
                        aMaterial->GetTexture(type, texIndex, &path);
                        mesh.materials[type].push_back(std::string(path.C_Str()));
                    }
                }
            }

            if (aScene->HasTextures())
            {
                for (unsigned int i = 0; i < aScene->mNumTextures; ++i)
                {
                    aiTexture* aTexture = aScene->mTextures[i];
                    m_textureDatas[i] = std::vector<unsigned char>(aTexture->mWidth);
                    TextureData& data = m_textureDatas[i];
                    std::memcpy(data.data(), aTexture->pcData, aTexture->mWidth);
                }
            }

            if (mesh.positions.empty())
            {
                printWarning("Invalid mesh (no vertices): " + file);
                return false;
            }

            size_t numVertices = mesh.positions.size();
            if (mesh.normals.size() != numVertices || mesh.tangents.size() != numVertices
                || mesh.uvs.size() != numVertices)
            {
                printWarning("Invalid mesh (vertex attribute sizes do not match): " + file);
                return false;
            }
            m_meshes.push_back(std::move(mesh));
        }
    }
    else
    {
        printWarning("Failed to read model: " + file);
        std::cerr << "Assimp error message: " << importer.GetErrorString() << "\n";
        return false;
    }

    if (m_meshes.empty())
    {
        printWarning("Empty model: " + file);
        return false;
    }

    printLog("Loaded model: " + file);
    return true;
}

const Model::Meshes& Model::getMeshes() const
{
    return m_meshes;
}

const Model::TextureData& Model::getTextureData(unsigned int index)
{
    return m_textureDatas[index];
}

} // namespace fw
