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

		void SetEntityID(uint64_t entityID)
		{
			int id = static_cast<int>(entityID);
			m_Model.InitializeID(id);
		}

		//void SetTextures(std::vector<std::pair<Texture, std::string>> t)
		//{
		//	m_Textures = t;
		//}

		void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 proj, 
			Shader other = Shader(), bool useDefault = true, int entityID = -1)
		{
			Shader shaderInUse = useDefault ? m_Shader : other;
			shaderInUse.Activate();

			if (m_DiffuseTex)
			{
				if (m_DiffuseTex->m_IsLoaded)
				{
					m_DiffuseTex->Bind(7);
					glUniform1i(glGetUniformLocation(shaderInUse.m_ID, "diffTex"), 7);
				}
			}

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

			std::vector<glm::vec3> test(m_Model.GetVertexCount(), glm::vec3(0.0f));
			VertexArray vao = m_Model.GetVAO();
			vao.Bind();

			std::vector<float> id(m_Model.GetVertexCount(), static_cast<float>(entityID));
			vao["EntityID"].Bind();
			vao["EntityID"].UpdateData<GLfloat>(0, static_cast<GLuint>(m_Model.GetVertexCount()), id.data());
			vao["EntityID"].Unbind();

			vao.Draw(GL_TRIANGLES, static_cast<GLuint>(m_Model.GetIndexCount()), GL_UNSIGNED_INT);
			vao.Clear();

			if (m_DiffuseTex)
			{
				if (m_DiffuseTex->m_IsLoaded)
				{
					m_DiffuseTex->Unbind();
				}
			}
		}

		void ReloadShader()
		{
			m_Shader.m_ID = 0;
			m_Shader.Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		}

		std::string GetVertexPath() const { return m_VertexSrc; }
		std::string GetFragmentPath() const { return m_FragmentSrc; }

		std::shared_ptr<Texture> GetDiffuseTex() { return m_DiffuseTex; }
		std::shared_ptr<Texture> GetNormalTex() { return m_NormalTex; }
		std::shared_ptr<Texture> GetMetallicTex() { return m_Metallic; }
		std::shared_ptr<Texture> GetRoughnessTex() { return m_Roughness; }

		std::string GetName() const { return m_Model.GetName(); }

	private:
		Model m_Model;
		Shader m_Shader;

		std::string m_VertexSrc = std::string(), m_FragmentSrc = std::string();

		std::shared_ptr<Texture> m_DiffuseTex;
		std::shared_ptr<Texture> m_NormalTex;
		std::shared_ptr<Texture> m_Metallic;
		std::shared_ptr<Texture> m_Roughness;

		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif