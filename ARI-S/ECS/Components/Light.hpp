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

		void Draw(glm::mat4 model, glm::mat4 view, glm::mat4 proj)
		{
			m_Shader.Activate();

			m_Shader.SetMat4("model", model);
			m_Shader.SetMat4("view", view);
			m_Shader.SetMat4("projection", proj);

			m_Light.GetVAO().Bind();
			m_Light.GetVAO().Draw(GL_TRIANGLES, static_cast<GLuint>(m_Light.GetIndexCount()), GL_UNSIGNED_INT);
			m_Light.GetVAO().Clear();
		}

		void ReloadShader()
		{
			m_Shader.m_ID = 0;
			m_Shader.Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		}

		std::string GetVertexPath() const { return m_VertexSrc; }
		std::string GetFragmentPath() const { return m_FragmentSrc; }

		glm::vec3 GetDirection() const { return m_Direction; }
		glm::vec4 GetColor() const { return m_Color; }

		float GetRange() const { return m_Range; }
		float GetIntensity() const { return m_Intensity; }

	private:
		Model m_Light;
		Shader m_Shader;

		std::string m_VertexSrc = std::string(), m_FragmentSrc = std::string();

		unsigned int m_Type = 0;
		glm::vec3 m_Direction = glm::vec3(0.0f);

		glm::vec4 m_Color = glm::vec4(1.0f);
		float m_Range = 10.0f;
		float m_Intensity = 1.0f;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif