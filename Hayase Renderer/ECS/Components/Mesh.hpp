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
		MeshComponent(const Model& model)
		{
			m_Model = model;
			m_Model.BuildArrays();
		}

		void operator=(const Model& model)
		{
			m_Model = model;

			if (m_Model.GetVAO().id != 0)
			{
				m_Model.DestroyArrays();
			}

			m_Model.BuildArrays();
		}

		std::string GetName() const { return m_Model.GetName(); }

	private:
		Model m_Model;

		std::vector<Texture> m_Textures;
		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif