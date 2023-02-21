#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <vector>
#include <glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Texture.h"
#include "Shader.h"
#include "ModelBuilder.h"

namespace ARIS
{
	class LightComponent
	{
	public:
		LightComponent()
		{
			ModelBuilder::CreateSphere(0.08f, 16, m_Light);
			m_Light.BuildArrays();

			ModelBuilder::CreateFrustum(m_Frustum);
			m_Frustum.BuildArrays();

			m_Shader = std::make_shared<Shader>(false, "CameraFrustum.vert", "CameraFrustum.frag");
		}

		LightComponent(const LightComponent& other) = default;

		template <typename T1, typename T2>
		void UpdateShader(T1 input1, T2 input2)
		{
			std::pair<T1, T2> res{ input1, input2 };
			//std::cout << res.first << " " << res.second << std::endl;
			m_Shader.Activate();
			m_Shader.SetData<T2>(res.first, res.second);
			glUseProgram(0);
		}
		
		// https://stackoverflow.com/questions/38370986/how-to-pass-variadic-amount-of-stdpair-with-different-2nd-types-to-a-functio
		template <typename T1, typename T2, typename... Args>
		void UpdateShader(T1 input1, T2 input2, Args... args)
		{
			std::pair<T1, T2> res{ input1, input2 };
			
			//std::cout << res.first << " " << res.second << std::endl;
			m_Shader.Activate();
			m_Shader.SetData<T2>(res.first, res.second);
			glUseProgram(0);

			UpdateShader(args...);
		}

		void Draw(glm::vec3 translation, glm::vec3 rotation, glm::mat4 v, glm::mat4 p)
		{
			m_Shader->Activate();

			
			glm::mat4 view = glm::lookAt(translation, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 proj = glm::ortho(-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
			glm::mat4 invProjView = glm::inverse(view);

			m_Shader->SetMat4("proj", p);
			m_Shader->SetMat4("view", v);

			if (m_Type == 0)
			{
				VertexArray vao = m_Frustum.GetVAO();
				vao.Bind();
				vao["Vertex"].Bind();
				
				float fov = 45.0f;
				float aspect = m_Width / m_Height;
				float halfHeight = tanf(glm::radians(fov / 2.0f));
				float halfWidth = halfHeight * aspect;
				
				float xn = halfWidth * m_Near;
				float xf = halfWidth * m_Far;
				float yn = halfHeight * m_Near;
				float yf = halfHeight * m_Far;
				
				std::vector<glm::vec3> newData =
				{
					// near face
					{xn, yn,   -m_Near},
					{-xn, yn,  -m_Near},
					{xn, -yn,  -m_Near},
					{-xn, -yn, -m_Near},
					
					// far face
					{xf, yf,   -m_Far},
					{-xf, yf,  -m_Far},
					{xf, -yf,  -m_Far},
					{-xf, -yf, -m_Far}
				};
				
				vao["Vertex"].UpdateData(0, static_cast<GLuint>(m_Frustum.GetVertexCount()), newData.data());

				vao.Draw(GL_LINES, static_cast<GLuint>(m_Frustum.GetIndexCount()), GL_UNSIGNED_INT);
				vao.Clear();
			}
			else
			{
				m_Light.GetVAO().Bind();
				m_Light.GetVAO().Draw(GL_TRIANGLES, static_cast<GLuint>(m_Light.GetIndexCount()), GL_UNSIGNED_INT);
				m_Light.GetVAO().Clear();
			}
		}

		void ReloadShader()
		{
			m_Shader->m_ID = 0;
			m_Shader->Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		}

		std::string GetVertexPath() const { return m_VertexSrc; }
		std::string GetFragmentPath() const { return m_FragmentSrc; }

		int GetType() const { return m_Type; }
		glm::vec4 GetColor() const { return m_Color; }

		float GetRange() const { return m_Range; }
		float GetIntensity() const { return m_Intensity; }

	private:
		Model m_Light, m_Frustum;
		std::shared_ptr<Shader> m_Shader;

		std::string m_VertexSrc = std::string(), m_FragmentSrc = std::string();

		// 0 = Directional
		unsigned int m_Type = 0;

		glm::vec4 m_Color = glm::vec4(1.0f);
		float m_Range = 10.0f;
		float m_Intensity = 1.0f;

		float m_Width = 10.0f;
		float m_Height = 10.0f;
		float m_Near = 0.1f;
		float m_Far = 30.0f;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif