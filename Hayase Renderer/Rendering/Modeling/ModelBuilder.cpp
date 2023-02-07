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
                    v.s_Normal =
                    {
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 0],
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 1],
                        attrib.normals[3 * static_cast<size_t>(index.normal_index) + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
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
                    model.m_Normals.push_back(v.s_Normal);
                    model.m_UVs.push_back(v.s_UV);
                }

                model.m_Indices.push_back(uniqueVerts[v]);
            }
        }
    }

    void ModelBuilder::LoadGLTF(std::string path, Model& model)
    {

    }
}