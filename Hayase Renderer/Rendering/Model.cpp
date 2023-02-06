#include <hyspch.h>
#include "Model.h"

#include "Texture.h"

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
	Model::Model(std::string path)
	{
		std::string fileType = std::string();

		size_t loc = path.find_last_of(".");
		if (loc != std::string::npos)
		{
			fileType = path.substr(loc + 1);
		}
		else
		{
			// warning
		}

		// probably a better way of doing this
		if (fileType.compare("obj") == 0)
		{
			ModelBuilder::LoadOBJ(path, *this);
		}
		else if (fileType.compare("gltf") == 0)
		{
			ModelBuilder::LoadGLTF(path, *this);
		}
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
                    uniqueVerts[v] = static_cast<uint32_t>(model.m_Vertices.size());
                    model.m_Vertices.push_back(v);
                }

                model.m_Indices.push_back(uniqueVerts[v]);
            }
        }
	}

	void ModelBuilder::LoadGLTF(std::string path, Model& model)
	{

	}
}