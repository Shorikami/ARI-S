#include <hyspch.h>
#include "ModelBuilder.h"

#include "Hasher.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

#include <tiny_obj_loader.h>

namespace std
{
    template<> struct hash<Hayase::Vertex>
    {
        std::size_t operator()(Hayase::Vertex const& v) const noexcept
        {
            std::size_t seed = 0;
            Hayase::HashCombine(seed, v.s_Position, v.s_Normal, v.s_UV, v.s_Tangent);
            return seed;
        }
    };
}

namespace Hayase
{

    ModelBuilder* ModelBuilder::m_Instance = nullptr;

    ModelBuilder::ModelBuilder()
    {
        m_Instance = this;
        BuildTable();
    }

    ModelBuilder::~ModelBuilder()
    {
        DestroyTable();
    }

    void ModelBuilder::BuildTable()
    {
        std::vector<std::string> lookUpExts =
        {
            "obj",
            "gltf"
        };

        // This is O(n^2); optimize?
        // Also this takes in a hard-coded content path. Pls change
        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("Content/Assets/Models"))
        {
            std::string filePath = dirEntry.path().string();

            for (std::string s : lookUpExts)
            {
                if (filePath.find(s) != std::string::npos)
                {
                    //std::cout << filePath << std::endl;
                    std::string name = std::string();
                    size_t nameLoc = filePath.find_last_of("\\");
                    size_t typeLoc = filePath.find_last_of(".");
                    
                    if (nameLoc != std::string::npos && typeLoc != std::string::npos)
                    {
                         name = filePath.substr(nameLoc + 1, typeLoc);
                         m_ModelTable[name] = new Model(filePath);
                         m_ModelTable[name]->m_Name = name;
                         //std::cout << name << std::endl;
                    }
                    else
                    {
                        // warning
                    }
                    
                }
            }
        }
    }

    void ModelBuilder::DestroyTable()
    {
        m_ModelTable.clear();
    }

    void ModelBuilder::LoadOBJ(std::string path, Model& model)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVerts{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex v{};
                bool vertexHasNorm = false;
                bool vertexHasUV = false;

                if (index.vertex_index >= 0)
                {
                    v.s_Position =
                    {
                        attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 0],
                        attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 1],
                        attrib.vertices[3 * static_cast<size_t>(index.vertex_index) + 2],
                    };
                }

                if (index.normal_index >= 0)
                {
                    vertexHasNorm = true;
                    v.s_Normal =
                    {
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0],
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1],
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertexHasUV = true;
                    v.s_UV =
                    {
                        attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 0],
                        attrib.texcoords[2 * static_cast<size_t>(index.texcoord_index) + 1],
                    };
                }

                if (uniqueVerts.count(v) == 0)
                {
                    uniqueVerts[v] = static_cast<uint32_t>(model.m_VertexData.size());
                    model.m_VertexData.push_back(v);

                    model.m_Vertices.push_back(v.s_Position);
                    if (vertexHasNorm)
                    {
                        model.m_Normals.push_back(v.s_Normal);
                    }
                    if (vertexHasUV)
                    {
                        model.m_UVs.push_back(v.s_UV);
                    }
                }

                model.m_Indices.push_back(uniqueVerts[v]);
            }
        }

        if (model.m_Normals.size() <= 0)
        {
            BuildNormals(model);
        }

        if (model.m_UVs.size() <= 0)
        {
            BuildTexCoords(model);
        }
    }

    void ModelBuilder::LoadGLTF(std::string path, Model& model)
    {

    }

    void ModelBuilder::BuildNormals(Model& model)
    {
        std::vector<glm::vec3> vvv;
        model.m_Normals.resize(model.m_Vertices.size(), glm::vec3(0.0f));

        // TODO: Manual normal generation
        uint32_t idx = 0;

        for (; idx < model.m_Indices.size();)
        {
            GLuint a = model.m_Indices.at(idx++);
            GLuint b = model.m_Indices.at(idx++);
            GLuint c = model.m_Indices.at(idx++);

            glm::vec3 vA = model.m_Vertices[a];
            glm::vec3 vB = model.m_Vertices[b];
            glm::vec3 vC = model.m_Vertices[c];

            glm::vec3 E1 = vB - vA;
            glm::vec3 E2 = vC - vA;

            glm::vec3 N = glm::normalize(glm::cross(E1, E2));

            if (N.x < 0.0001f && N.x > -0.0001f)
            {
                N.x = 0.0f;
            }
            if (N.y < 0.0001f && N.y > -0.0001f)
            {
                N.y = 0.0f;
            }
            if (N.z < 0.0001f && N.z > -0.0001f)
            {
                N.z = 0.0f;
            }

            vvv.push_back(N);
        }

        for (int idx = 0; idx < model.m_Vertices.size(); ++idx)
        {
            glm::vec3 vNormal(0.0f);

            int bb = 0;
            std::vector<glm::vec3> dup;

            for (int kk = 0; bb < model.m_Indices.size(); ++kk)
            {
                GLuint a = model.m_Indices.at(bb++);
                GLuint b = model.m_Indices.at(bb++);
                GLuint c = model.m_Indices.at(bb++);

                if (a == idx || b == idx || c == idx)
                {
                    bool isDup = false;

                    for (int k = 0; k < dup.size(); ++k)
                    {
                        if (vvv[kk] == dup[k])
                        {
                            isDup = true;
                        }
                    }

                    if (!isDup)
                    {
                        dup.push_back(vvv[kk]);
                        vNormal += vvv[kk];
                    }
                }
            }

            model.m_Normals[idx] = glm::normalize(vNormal);
        }
    }

    void ModelBuilder::BuildTexCoords(Model& model)
    {
    
    }
}