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

		void SetTextures(std::vector< std::pair<Texture, std::string>> t)
		{
			m_Textures = t;
		}

		void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 proj, Shader other = Shader(), bool useDefault = true)
		{
			if (useDefault)
			{
				m_Shader.Activate();
			}
			else
			{
				other.Activate();
			}
			

			for (unsigned i = 0; i < m_Textures.size(); ++i)
			{
				m_Textures[i].first.Bind(i);

				if (useDefault)
				{
					glUniform1i(glGetUniformLocation(m_Shader.m_ID, m_Textures[i].second.c_str()), i);
				}
				else
				{
					glUniform1i(glGetUniformLocation(other.m_ID, m_Textures[i].second.c_str()), i);
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

			m_Model.GetVAO().Bind();
			m_Model.GetVAO().Draw(GL_TRIANGLES, static_cast<GLuint>(m_Model.GetIndexCount()), GL_UNSIGNED_INT);
			m_Model.GetVAO().Clear();
		}

		void ReloadShader()
		{
			m_Shader.m_ID = 0;
			m_Shader.Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		}

		std::string GetVertexPath() const { return m_VertexSrc; }
		std::string GetFragmentPath() const { return m_FragmentSrc; }

		std::string GetName() const { return m_Model.GetName(); }

	private:
		Model m_Model;
		Shader m_Shader;

		std::string m_VertexSrc = std::string(), m_FragmentSrc = std::string();

		std::vector<std::pair<Texture, std::string>> m_Textures;
		glm::vec4 m_Ambient, m_Albedo, m_Specular;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif