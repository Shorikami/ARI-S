#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <glm.hpp>

#include "VertexMemory.hpp"
#include "Texture.h"

#include "Model.h"

namespace Hayase
{
	class MeshComponent
	{
	public:
		MeshComponent() = default;
		MeshComponent(std::string path)
		{
		}
	private:
		Model m;

		std::vector<Texture> m_Textures;
		glm::vec4 m_Ambient, m_Albedo, m_Specular;
	};
}

#endif