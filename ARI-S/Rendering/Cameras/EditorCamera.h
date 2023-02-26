#ifndef EDITORCAMERA_H
#define EDITORCAMERA_H

#include "Camera.h"
#include "AppEvent.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#include "Timer.h"

namespace ARIS
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float n, float f);

		void OnUpdate(DeltaTime dt);
		void OnEvent(Event& e);

		__inline float GetDistance() const { return m_Distance; }
		__inline void SetDistance(float D) { m_Distance = D; }

		__inline void SetViewportSize(int w, int h) { m_ViewportWidth = w; m_ViewportHeight = h; UpdateProjection(); }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }

	private:
		void UpdateProjection();
		void UpdateView();

		void MoveFocalPoint(CameraDirection d, DeltaTime dt);

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& dt);
		void MouseRotate(const glm::vec2& dt);
		void MouseZoom(float dt);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		float m_FOV = 45.0f;
		float m_AspectRatio = 1.778f;
		float m_Near = 0.1f, m_Far = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_FocalPoint = glm::vec3(0.0f);

		glm::vec2 m_InitialMouse = glm::vec2(0.0f);

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		int m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}


#endif