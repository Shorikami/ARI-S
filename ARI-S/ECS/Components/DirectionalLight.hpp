#ifndef DIRECTIONLIGHT_HPP
#define DIRECTIONLIGHT_HPP

#include <vector>
#include <glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Texture.h"
#include "Shader.h"
#include "ModelBuilder.h"

namespace ARIS
{
	class DirectionLightComponent
	{
	public:
		DirectionLightComponent()
			: m_Color(glm::vec4(1.0f))
			, m_View(glm::mat4(1.0f))
			, m_Projection(glm::mat4(1.0f))
		{
			ModelBuilder::CreateSphere(0.08f, 16, m_Light);
			m_Light.BuildArrays();
		}

		DirectionLightComponent(const DirectionLightComponent& other) = default;

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

		//void ReloadShader()
		//{
		//	m_Shader->m_ID = 0;
		//	m_Shader->Generate(false, m_VertexSrc.c_str(), m_FragmentSrc.c_str());
		//}

		void Update(glm::vec3 position, glm::vec3 rotation)
		{
			glm::vec3 dir = glm::vec3(0.0f);
			dir.x = cos(rotation.x) * cos(rotation.y);
			dir.y = sin(rotation.y);
			dir.z = sin(rotation.x) * cos(rotation.y);
			dir = glm::normalize(dir);

			m_View = glm::lookAt(position, position + dir, glm::vec3(0.0f, 1.0f, 0.0f));

			m_Projection = glm::ortho(-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
		}

		glm::vec4 GetColor() const { return m_Color; }

		glm::mat4 GetViewMatrix() { return m_View; }
		glm::mat4 GetProjectionMatrix() { return m_Projection; }

		float GetWidth() const { return m_Width; }
		float GetHeight() const { return m_Height; }
		float GetNear() const { return m_Near; }
		float GetFar() const { return m_Far; }

	private:
		Model m_Light;
		std::shared_ptr<Shader> m_Shader;

		glm::vec4 m_Color = glm::vec4(1.0f);

		glm::mat4 m_View;
		glm::mat4 m_Projection;

		float m_Width = 10.0f;
		float m_Height = 10.0f;
		float m_Near = 0.1f;
		float m_Far = 25.0f;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif