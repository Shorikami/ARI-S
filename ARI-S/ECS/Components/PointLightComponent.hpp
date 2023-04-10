#ifndef POINTLIGHT_HPP
#define POINTLIGHT_HPP

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
	class PointLightComponent
	{
	public:
		PointLightComponent()
			: m_Color(glm::vec4(1.0f))
			, m_Intensity(1.0f)
			, m_Range(10.0f)
		{
			m_Light = ModelBuilder::Get().CreateSphere(1.0f, 16);
			m_Shader = std::make_shared<Shader>(true, "IBL/LocalLightPBR.vert", "IBL/LocalLightPBR.frag", nullptr, "IBL/FormulasIBL.gh");
		}

		PointLightComponent(const PointLightComponent& other) = default;

		template <typename T1, typename T2>
		void UpdateShader(T1 input1, T2 input2)
		{
			std::pair<T1, T2> res{ input1, input2 };
			//std::cout << res.first << " " << res.second << std::endl;
			m_Shader->Activate();
			m_Shader->SetData<T2>(res.first, res.second);
			glUseProgram(0);
		}

		// https://stackoverflow.com/questions/38370986/how-to-pass-variadic-amount-of-stdpair-with-different-2nd-types-to-a-functio
		template <typename T1, typename T2, typename... Args>
		void UpdateShader(T1 input1, T2 input2, Args... args)
		{
			std::pair<T1, T2> res{ input1, input2 };

			//std::cout << res.first << " " << res.second << std::endl;
			m_Shader->Activate();
			m_Shader->SetData<T2>(res.first, res.second);
			glUseProgram(0);

			UpdateShader(args...);
		}

		void Draw(glm::vec3 position, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
		{
			m_Shader->Activate();

			m_Shader->SetMat4("model", model);
			m_Shader->SetMat4("view", view);
			m_Shader->SetMat4("projection", projection);
		
			// render the PBR effect on objects
			m_Light->Draw(*m_Shader.get());

			// render the actual light (debug)
			dd::sphere(glm::value_ptr(position), glm::value_ptr(m_Color), 0.1f);

			if (ModelBuilder::Get().m_DisplayBoxes)
			{
				dd::sphere(glm::value_ptr(position), glm::value_ptr(m_Color), m_Range);
			}
		}

		void ReloadShader()
		{
			m_Shader->m_ID = 0;
			m_Shader->Generate(true, m_Shader->m_VertPath.c_str(), m_Shader->m_FragPath.c_str(), nullptr, "IBL/FormulasIBL.gh");
		}

		glm::vec4 GetColor() const { return m_Color; }

		float GetRange() const { return m_Range; }
		float GetIntensity() const { return m_Intensity; }
	private:
		Model* m_Light;
		std::shared_ptr<Shader> m_Shader;
		glm::vec4 m_Color = glm::vec4(1.0f);

		float m_Range = 10.0f;
		float m_Intensity = 1.0f;

		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif