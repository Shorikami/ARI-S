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

		void ReloadShader()
		{
			m_Shader.m_ID = 0;
			m_Shader.Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		}

		std::string GetVertexPath() const { return m_VertexSrc; }
		std::string GetFragmentPath() const { return m_FragmentSrc; }

		Texture* GetDiffuseTex() { return m_DiffuseTex; }
		Texture* GetNormalTex() { return m_NormalTex; }
		Texture* GetMetallicTex() { return m_Metallic; }
		Texture* GetRoughnessTex() { return m_Roughness; }
		Texture* GetMetalRough() { return m_MetalRough; }

		std::string GetName() const { return m_Model->GetName(); }
		std::string GetPath() const { return m_Model->GetPath(); }

		void SetName(std::string s) { m_Model->SetName(s); }
		void SetPath(std::string s) { m_Model->SetPath(s); }

	private:
		Model* m_Model;
		Shader m_Shader;

		std::string m_VertexSrc = std::string(), m_FragmentSrc = std::string();

		Texture* m_DiffuseTex;
		Texture* m_NormalTex;
		Texture* m_Metallic;
		Texture* m_Roughness;
		Texture* m_MetalRough;

		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif