#include <arpch.h>
#include "ModelBuilder.h"
#include "Texture.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

namespace ARIS
{

    ModelBuilder* ModelBuilder::m_Instance = nullptr;

    ModelBuilder::ModelBuilder()
    {
        m_Instance = this;
    }

    ModelBuilder::~ModelBuilder()
    {
        DestroyTable();
    }

    void ModelBuilder::DestroyTable()
    {
        m_ModelTable.clear();
    }

    Model* ModelBuilder::LoadModel(std::string path)
    {
        for (Model* m : m_ModelTable)
        {
            if (m->m_Path.compare(path) == 0)
            {
                return new Model(*m);
            }
        }
        Model* m = new Model();
        m->m_Path = path;

        GenerateModel(path, *m);
        m_ModelTable.push_back(m);

        return m;
    }

    void ModelBuilder::GenerateModel(std::string path, Model& model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
            aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_JoinIdenticalVertices | aiProcess_GenBoundingBoxes);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE | !scene->mRootNode)
        {
            std::cout << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
            return;
        }

        ProcessNode(scene->mRootNode, scene, model);
    }

    void ModelBuilder::ProcessNode(aiNode* node, const aiScene* scene, Model& model)
    {
        for (unsigned i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.m_Meshes.push_back(ProcessMesh(mesh, scene, model));
        }

        for (unsigned i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    Mesh ModelBuilder::ProcessMesh(aiMesh* mesh, const aiScene* scene, Model& model)
    {
        std::vector<Vertex> vertexData;
        std::vector<unsigned> indices;
        std::vector<Texture> textures;

        glm::vec3 maxBB;
        glm::vec3 minBB;

        for (unsigned i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex v;
            
            v.s_Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals())
            {
                v.s_Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }

            if (mesh->mTextureCoords[0])
            {
                v.s_UV = glm::vec3(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, mesh->mTextureCoords[0][i].z);
                v.s_Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                v.s_Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            }
            else
            {
                v.s_UV = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            vertexData.push_back(v);
        }

        for (unsigned i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        maxBB = glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
        minBB = glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, model);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, model);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, model);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> metalMap = LoadMaterialTextures(material, aiTextureType_METALNESS, model);
        textures.insert(textures.end(), metalMap.begin(), metalMap.end());

        std::vector<Texture> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, model);
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, model);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        std::vector<Texture> metalRoughMaps = LoadMaterialTextures(material, aiTextureType_UNKNOWN, model);
        textures.insert(textures.end(), metalRoughMaps.begin(), metalRoughMaps.end());

        return Mesh(vertexData, indices, textures, maxBB, minBB);
    }

    std::vector<Texture> ModelBuilder::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, Model& model)
    {
        std::vector<Texture> t;

        for (unsigned i = 0; i < mat->GetTextureCount(type); ++i)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned j = 0; j < model.m_LoadedTextures.size(); ++j)
            {
                size_t remove = model.m_LoadedTextures[j].m_Path.find_last_of("/\\");
                std::string path = model.m_LoadedTextures[j].m_Path.substr(remove, model.GetPath().length());

                std::string aiStringStr = std::string(str.C_Str());
                std::string aiStringCmp = std::string();

                remove = aiStringStr.find_last_of("/\\");
                if (remove > model.m_LoadedTextures[j].m_Path.length() || remove < 0)
                {
                    aiStringCmp = aiStringStr;
                }
                else
                {
                    aiStringCmp = aiStringStr.substr(remove, aiStringStr.length());
                }

                if (std::strcmp(path.c_str(), aiStringCmp.c_str()) == 0)
                {
                    t.push_back(model.m_LoadedTextures[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                size_t remove = model.GetPath().find_last_of("/\\");
                std::string dir = model.GetPath().substr(0, remove) + std::string("/\\");

                Texture newTex(dir + std::string(str.C_Str()), GL_LINEAR, GL_REPEAT, false, type);
                t.push_back(newTex);
                model.m_LoadedTextures.push_back(newTex);
            }
        }

        return t;
    }

    Model* ModelBuilder::CreateSphere(float radius, unsigned divisions)
    {
        Mesh m;

        float x, y, z, xy;
        float length = 1.0f / radius;

        constexpr float pi = glm::pi<float>();

        float sectorStep = 2 * pi / divisions;
        float stackStep = pi / divisions;
        float sectorAngle, stackAngle;

        for (unsigned i = 0; i <= divisions; ++i)
        {
            stackAngle = pi / 2 - i * stackStep;
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (unsigned j = 0; j <= divisions; ++j)
            {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                float nx = x * length;
                float ny = y * length;
                float nz = z * length;

                Vertex v;
                v.s_Position = glm::vec3(x, y, z);
                v.s_Normal = glm::vec3(nx, ny, nz);

                m.m_VertexData.push_back(v);
            }
        }

        unsigned k1, k2;
        for (unsigned i = 0; i < divisions; ++i)
        {
            k1 = i * (divisions + 1);
            k2 = k1 + divisions + 1;

            for (unsigned j = 0; j < divisions; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    m.m_Indices.push_back(k1);
                    m.m_Indices.push_back(k2);
                    m.m_Indices.push_back(k1 + 1);
                }

                if (i != (divisions - 1))
                {
                    m.m_Indices.push_back(k1 + 1);
                    m.m_Indices.push_back(k2);
                    m.m_Indices.push_back(k2 + 1);
                }
            }
        }

        Model* mo = new Model();
        mo->m_Meshes.push_back(m);

        return mo;
    }
}