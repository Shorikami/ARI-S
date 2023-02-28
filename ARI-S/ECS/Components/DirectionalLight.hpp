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
			m_Shader = std::make_shared<Shader>(false, "LineShader.vert", "LineShader.frag");

			std::vector<glm::vec3> vertices =
			{
				glm::vec3(0.0f),
				glm::vec3(0.0f)
			};

			glGenVertexArrays(1, &m_VAO);
			glGenBuffers(1, &m_VBO);
			glBindVertexArray(m_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

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
			//glm::vec3 dir = glm::vec3(0.0f);
			//dir.x = cos(rotation.x) * cos(rotation.y);
			//dir.y = sin(rotation.y);
			//dir.z = sin(rotation.x) * cos(rotation.y);
			//dir = glm::normalize(dir);
			//
			//m_View = glm::lookAt(position, position + dir, glm::vec3(0.0f, 1.0f, 0.0f));

			//std::cout << "{ " << rotation.x << " " << rotation.y << " " << rotation.z << " }" << std::endl;

			glm::quat orientation = glm::quat(glm::vec3(-rotation.x, -rotation.y, 0.0f));
			m_View = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
			m_View = glm::inverse(m_View);

			//m_View = 
			//{
			//	glm::vec4(right.x, up.x, forward.x, 0.0f),
			//	glm::vec4(right.y, up.y, forward.y, 0.0f),
			//	glm::vec4(right.z, up.z, forward.z, 0.0f),
			//	glm::vec4(-glm::dot(right, position), -glm::dot(up, position), -glm::dot(forward, position), 1.0f)
			//};

			if (m_UsePerspective)
			{
				m_Projection = glm::perspective(glm::radians(30.0f), 1.0f, m_Near, m_Far);
			}
			else
			{
				m_Projection = glm::ortho(-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
			}
		}

		void Draw(glm::vec3 position, glm::vec3 forward, glm::mat4 view, glm::mat4 proj)
		{
			m_Shader->Activate();

			m_Shader->SetMat4("view", view);
			m_Shader->SetMat4("projection", proj);

			glBindVertexArray(m_VAO);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

			std::vector<glm::vec3> newNorm =
			{
				position,
				position + (forward * 2.0f)
			};

			glBufferSubData(GL_ARRAY_BUFFER, 0, newNorm.size() * sizeof(glm::vec3), newNorm.data());
			
			glDrawArrays(GL_LINES, 0, 2);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(0);
			glUseProgram(0);
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

		GLuint m_VAO;
		GLuint m_VBO;

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