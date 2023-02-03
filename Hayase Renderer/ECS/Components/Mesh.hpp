#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <glm.hpp>

#include "VertexMemory.hpp"
#include "Texture.h"

namespace Hayase
{
	struct Vertex
	{
		glm::vec3 s_Position;
		glm::vec3 s_Normal;
		glm::vec2 s_UV;
		glm::vec3 s_Tangent;
	};

	class MeshComponent
	{
	public:
		MeshComponent() = default;
		MeshComponent(std::string path)
		{
			
		}
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;
		std::vector<Texture> m_Textures;

		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		VertexArray m_MeshArray;
	};
}

#endif