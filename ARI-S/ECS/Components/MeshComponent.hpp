#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "VertexMemory.hpp"
#include "Texture.h"

#include "Model.h"
#include "Shader.h"

namespace ARIS
{
	class MeshComponent
	{
	public:
		MeshComponent() = default;

		MeshComponent(const Model& model)
		{
			*m_Model = model;
		}

		void operator=(const Model& model)
		{
			*m_Model = model;
		}

		void SetEntityID(uint64_t entityID)
		{
			int id = static_cast<int>(entityID);
			m_Model->InitializeID(id);
		}

		void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 proj, 
			Shader other = Shader(), bool useDefault = true, int entityID = -1)
		{
			if (!m_Model)
				return;

			Shader shaderInUse = useDefault ? m_Shader : other;
			shaderInUse.Activate();

			if (useDefault)
			{
				m_Shader.SetMat4("model", model);
				m_Shader.SetMat4("view", view);
				m_Shader.SetMat4("projection", proj);
			}
			else
			{
				other.SetMat4("model", model);
				other.SetMat4("view", view);
				other.SetMat4("projection", proj);
			}

			m_Model->Draw(shaderInUse, entityID);
		}

		Texture* GetDiffuseTex() { return m_DiffuseTex; }
		Texture* GetNormalTex() { return m_NormalTex; }
		Texture* GetMetallicTex() { return m_MetallicTex; }
		Texture* GetRoughnessTex() { return m_RoughnessTex; }
		Texture* GetMetalRough() { return m_MetalRoughTex; }

		std::string GetName() const { return m_Model->GetName(); }
		std::string GetPath() const { return m_Model->GetPath(); }

		void SetName(std::string s) { m_Model->SetName(s); }
		void SetPath(std::string s) { m_Model->SetPath(s); }

		bool& GetControllableMetRough() { return m_ControllableMetalRoughness; }

		float& GetMetalness() { return m_Metalness; }
		float& GetRoughness() { return m_Roughness; }

	private:
		Model* m_Model;
		Shader m_Shader;

		Texture* m_DiffuseTex;
		Texture* m_NormalTex;
		Texture* m_MetallicTex;
		Texture* m_RoughnessTex;
		Texture* m_MetalRoughTex;

		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		bool m_ControllableMetalRoughness;
		float m_Metalness;
		float m_Roughness;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif