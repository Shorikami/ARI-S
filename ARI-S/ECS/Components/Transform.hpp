#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

namespace ARIS
{
	class TransformComponent
	{
	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		
		glm::vec3 GetTranslation() const { return m_Translation; }
		glm::vec3 GetRotation() const { return m_Rotation; }
		glm::vec3 GetScale() const { return m_Scale; }
		glm::quat GetOrientation() const { return glm::quat(glm::vec3(m_Rotation.x, m_Rotation.y, 0.0f)); }

		glm::vec3 Forward() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, 1.0f)); }
		glm::vec3 Up() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
		glm::vec3 Right() const { return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }

		void Translate(glm::vec3 v) { m_Translation = v; }
		void Rotate(glm::vec3 v) { m_Rotation = v; }
		void Scale(glm::vec3 v) { m_Scale = v; }

		glm::mat4 GetTransform() const { return m_TransformMatrix; }

		void Update()
		{
			m_TransformMatrix = glm::translate(glm::mat4(1.0f), m_Translation)
				* glm::toMat4(glm::quat(m_Rotation))
				* glm::scale(glm::mat4(1.0f), m_Scale);
		}

	private:
		glm::vec3 m_Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };

		glm::mat4 m_TransformMatrix = glm::mat4(1.0f);

		friend class SceneSerializer;
		friend class HierarchyPanel;
		friend class Editor;
	};
}

#endif