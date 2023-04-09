#ifndef DIRECTIONLIGHT_HPP
#define DIRECTIONLIGHT_HPP

#include <vector>
#include <glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Texture.h"
#include "Shader.h"
#include "ModelBuilder.h"
#include "../Rendering/DebugDraw.h"

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
			m_Shader = std::make_shared<Shader>(false, "LineShader.vert", "LineShader.frag");
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

		void Update(glm::vec3 position, glm::vec3 rotation)
		{

			glm::quat orientation = glm::quat(glm::vec3(rotation.x, rotation.y, 0.0f));
			m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
			m_View = glm::inverse(m_View);

			if (m_UsePerspective)
			{
				m_Projection = glm::perspective(glm::radians(30.0f), 1.0f, m_Near, m_Far);
			}
			else
			{
				m_Projection = glm::ortho(-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
			}
		}

		void Draw(glm::vec3 position, glm::vec3 forward, glm::mat4 p, glm::mat4 v)
		{
			dd::frustum(glm::value_ptr(glm::inverse(p * v)), glm::value_ptr(m_Color));
			//dd::vertexNormal(glm::value_ptr(position), glm::value_ptr(forward), 1.0f);
			//dd::circle(glm::value_ptr(position), glm::value_ptr(forward), glm::value_ptr(glm::vec4(m_Color)), 0.5f, 36.0f);
		}

		glm::vec4 GetColor() const { return m_Color; }

		glm::mat4 GetViewMatrix() { return m_View; }
		glm::mat4 GetProjectionMatrix() { return m_Projection; }

		float GetWidth() const { return m_Width; }
		float GetHeight() const { return m_Height; }
		float GetNear() const { return m_Near; }
		float GetFar() const { return m_Far; }

		bool GetPerspectiveInUse() const { return m_UsePerspective; }

	private:
		std::shared_ptr<Shader> m_Shader;

		glm::vec4 m_Color = glm::vec4(1.0f);

		glm::mat4 m_View;
		glm::mat4 m_Projection;

		float m_Width = 10.0f;
		float m_Height = 10.0f;
		float m_Near = 0.1f;
		float m_Far = 25.0f;

		bool m_UsePerspective = false;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif