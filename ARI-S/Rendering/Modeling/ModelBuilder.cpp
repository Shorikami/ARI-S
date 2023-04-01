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

    void ModelBuilder::LoadModel(std::string path, Model& model)
    {
        for (Model* m : m_ModelTable)
        {
            if (m->m_Path.compare(path) == 0)
            {
                model = *m;
                return;
            }
        }
        GenerateModel(path, model);
        m_ModelTable.push_back(&model);
    }

    void ModelBuilder::GenerateModel(std::string path, Model& model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
            aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices | aiProcess_JoinIdenticalVertices);

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

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, model);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, model);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, model);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, model);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertexData, indices, textures);
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
                if (std::strcmp(model.m_LoadedTextures[j].m_Path.data(), str.C_Str()) == 0)
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

    void ModelBuilder::CreateSphere(float radius, unsigned divisions, Model& model)
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

                m.GetVertexData().push_back(v);
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
                    m.GetIndices().push_back(k1);
                    m.GetIndices().push_back(k2);
                    m.GetIndices().push_back(k1 + 1);
                }

                if (i != (divisions - 1))
                {
                    m.GetIndices().push_back(k1 + 1);
                    m.GetIndices().push_back(k2);
                    m.GetIndices().push_back(k2 + 1);
                }
            }
        }

        model.m_Meshes.push_back(m);
    }

    //void ModelBuilder::LoadOBJ(std::string path, Model& model)
    //{
    //    tinyobj::attrib_t attrib;
    //    std::vector<tinyobj::shape_t> shapes;
    //    std::vector<tinyobj::material_t> materials;
    //    std::string warn, err;
    //
    //    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    //    {
    //        throw std::runtime_error(warn + err);
    //    }
    //
    //    std::unordered_map<Vertex, uint32_t> uniqueVerts{};
    //
    //    for (const auto& shape : shapes)
    //    {
    //        for (const auto& index : shape.mesh.indices)
    //        {
    //            Vertex v{};
    //            bool vertexHasNorm = false;
    //            bool vertexHasUV = false;
    //
    //            if (index.vertex_index >= 0)
    //            {
    //                v.s_Position =
    //                {
    //                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 0],
    //                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
    //                    attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2],
    //                };
    //            }
    //
    //            if (index.normal_index >= 0)
    //            {
    //                vertexHasNorm = true;
    //                v.s_Normal =
    //                {
    //                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0],
    //                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1],
    //                    attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2],
    //                };
    //            }
    //
    //            if (index.texcoord_index >= 0)
    //            {
    //                vertexHasUV = true;
    //                v.s_UV =
    //                {
    //                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0],
    //                    attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1],
    //                };
    //            }
    //
    //            if (uniqueVerts.count(v) == 0)
    //            {
    //                uniqueVerts[v] = static_cast<uint32_t>(model.m_VertexData.size());
    //                model.m_VertexData.push_back(v);
    //
    //                model.m_Vertices.push_back(v.s_Position);
    //                if (vertexHasNorm)
    //                {
    //                    model.m_Normals.push_back(v.s_Normal);
    //                }
    //                if (vertexHasUV)
    //                {
    //                    model.m_UVs.push_back(v.s_UV);
    //                }
    //            }
    //
    //            model.m_Indices.push_back(uniqueVerts[v]);
    //        }
    //    }
    //
    //    if (model.m_Normals.size() <= 0)
    //    {
    //        BuildNormals(model);
    //    }
    //
    //    if (model.m_UVs.size() <= 0)
    //    {
    //        BuildTexCoords(model);
    //    }
    //}
    //
    //void ModelBuilder::LoadGLTF(std::string path, Model& model)
    //{
    //
    //}
    //
    //void ModelBuilder::BuildNormals(Model& model)
    //{
    //    std::vector<glm::vec3> vvv;
    //    model.m_Normals.resize(model.m_Vertices.size(), glm::vec3(0.0f));
    //
    //    // TODO: Manual normal generation
    //    uint32_t idx = 0;
    //
    //    for (; idx < model.m_Indices.size();)
    //    {
    //        GLuint a = model.m_Indices.at(idx++);
    //        GLuint b = model.m_Indices.at(idx++);
    //        GLuint c = model.m_Indices.at(idx++);
    //
    //        glm::vec3 vA = model.m_Vertices[a];
    //        glm::vec3 vB = model.m_Vertices[b];
    //        glm::vec3 vC = model.m_Vertices[c];
    //
    //        glm::vec3 E1 = vB - vA;
    //        glm::vec3 E2 = vC - vA;
    //
    //        glm::vec3 N = glm::normalize(glm::cross(E1, E2));
    //
    //        if (N.x < 0.0001f && N.x > -0.0001f)
    //        {
    //            N.x = 0.0f;
    //        }
    //        if (N.y < 0.0001f && N.y > -0.0001f)
    //        {
    //            N.y = 0.0f;
    //        }
    //        if (N.z < 0.0001f && N.z > -0.0001f)
    //        {
    //            N.z = 0.0f;
    //        }
    //
    //        vvv.push_back(N);
    //    }
    //
    //    for (int idx = 0; idx < model.m_Vertices.size(); ++idx)
    //    {
    //        glm::vec3 vNormal(0.0f);
    //
    //        int bb = 0;
    //        std::vector<glm::vec3> dup;
    //
    //        for (int kk = 0; bb < model.m_Indices.size(); ++kk)
    //        {
    //            GLuint a = model.m_Indices.at(bb++);
    //            GLuint b = model.m_Indices.at(bb++);
    //            GLuint c = model.m_Indices.at(bb++);
    //
    //            if (a == idx || b == idx || c == idx)
    //            {
    //                bool isDup = false;
    //
    //                for (int k = 0; k < dup.size(); ++k)
    //                {
    //                    if (vvv[kk] == dup[k])
    //                    {
    //                        isDup = true;
    //                    }
    //                }
    //
    //                if (!isDup)
    //                {
    //                    dup.push_back(vvv[kk]);
    //                    vNormal += vvv[kk];
    //                }
    //            }
    //        }
    //
    //        model.m_Normals[idx] = glm::normalize(vNormal);
    //    }
    //}
    //
    //void ModelBuilder::BuildTexCoords(Model& model)
    //{
    //    // Planar mapping only; TODO: Add some more
    //    size_t vertexCount = model.m_Vertices.size();
    //    for (size_t v = 0; v < vertexCount; ++v)
    //    {
    //        glm::vec3 V = model.m_Vertices[v];
    //        glm::vec2 uv(0.0f);
    //
    //        float min = -1.0f, max = 1.0f;
    //
    //        uv.x = (V.x - min) / (max - min);
    //        uv.y = (V.y - min) / (max - min);
    //
    //        model.m_UVs.push_back(uv);
    //    }
    //}
}