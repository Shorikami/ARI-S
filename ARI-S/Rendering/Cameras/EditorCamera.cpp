#include <arpch.h>
#include "EditorCamera.h"

#include "InputPoll.h"
#include "Tools.h"

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

namespace ARIS
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float n, float f)
		: m_FOV(fov)
		, m_AspectRatio(aspectRatio)
		, m_Near(n)
		, m_Far(f)
		, Camera(glm::perspective(glm::radians(fov), aspectRatio, n, f))
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(DeltaTime dt)
	{
		if (InputPoll::IsKeyPressed(KeyTags::LeftAlt))
		{
			const glm::vec2& mouse{ InputPoll::GetMousePosition().x, InputPoll::GetMousePosition().y };
			glm::vec2 delta = (mouse - m_InitialMouse) * 0.003f;
			m_InitialMouse = mouse;

			if (InputPoll::IsMouseButtonPressed(MouseTags::ButtonMiddle))
				MousePan(delta);
			else if (InputPoll::IsMouseButtonPressed(MouseTags::ButtonLeft))
				MouseRotate(delta);
			else if (InputPoll::IsMouseButtonPressed(MouseTags::ButtonRight))
				MouseZoom(delta.y);
		}

		else if (InputPoll::IsMouseButtonPressed(MouseTags::ButtonRight))
		{
			if (InputPoll::IsKeyPressed(KeyTags::W))
				MoveFocalPoint(CameraDirection::FORWARD, dt);
			if (InputPoll::IsKeyPressed(KeyTags::S))
				MoveFocalPoint(CameraDirection::BACKWARDS, dt);
			if (InputPoll::IsKeyPressed(KeyTags::A))
				MoveFocalPoint(CameraDirection::LEFT, dt);
			if (InputPoll::IsKeyPressed(KeyTags::D))
				MoveFocalPoint(CameraDirection::RIGHT, dt);
			if (InputPoll::IsKeyPressed(KeyTags::Q))
				MoveFocalPoint(CameraDirection::UP, dt);
			if (InputPoll::IsKeyPressed(KeyTags::E))
				MoveFocalPoint(CameraDirection::DOWN, dt);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNC(EditorCamera::OnMouseScroll));
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_Near, m_Far);
	}

	void EditorCamera::UpdateView()
	{
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void EditorCamera::MoveFocalPoint(CameraDirection d, DeltaTime dt)
	{
		float velocity = dt.GetSeconds() * 10.0f;
		switch (d)
		{
		case CameraDirection::FORWARD:
			m_FocalPoint += GetForwardDirection() * velocity;
			break;
		case CameraDirection::BACKWARDS:
			m_FocalPoint -= GetForwardDirection() * velocity;
			break;
		case CameraDirection::RIGHT:
			m_FocalPoint += GetRightDirection() * velocity;
			break;
		case CameraDirection::LEFT:
			m_FocalPoint -= GetRightDirection() * velocity;
			break;
		case CameraDirection::UP:
			m_FocalPoint += GetUpDirection() * velocity;
			break;
		case CameraDirection::DOWN:
			m_FocalPoint -= GetUpDirection() * velocity;
			break;
		}
	
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& dt)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * dt.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * dt.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& dt)
	{
		float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
		m_Yaw += yawSign * dt.x * RotationSpeed();
		m_Pitch += dt.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float dt)
	{
		m_Distance -= dt * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}
}