#include <arpch.h>

#include "Event.h"
#include "Camera.h"
#include "InputPoll.h"
#include "Tools.h"

#include <cmath>

namespace ARIS
{
	Camera::Camera(glm::vec3 pos)
		: cameraPos(pos)
		, yaw(0.0f)
		, pitch(0.0f)
		, speed(2.5f)
		, zoom(45.0f)
		, front(glm::vec3(0.0f, 0.0f, -1.0f))
		, n(0.5f)
		, f(100.0f)
		, lastX(0.0f)
		, lastY(0.0f)
		, firstMouse(true)
	{
		UpdateCameraVectors();
	}

	void Camera::Update(DeltaTime dt)
	{
		double time = static_cast<double>(dt.GetSeconds());

		if (enableCameraMovement)
		{
			if (InputPoll::IsKeyPressed(KeyTags::W))
				UpdateCameraPos(CameraDirection::FORWARD, time);
			if (InputPoll::IsKeyPressed(KeyTags::S))
				UpdateCameraPos(CameraDirection::BACKWARDS, time);
			if (InputPoll::IsKeyPressed(KeyTags::A))
				UpdateCameraPos(CameraDirection::LEFT, time);
			if (InputPoll::IsKeyPressed(KeyTags::D))
				UpdateCameraPos(CameraDirection::RIGHT, time);
		}	
	}

	void Camera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FUNC(Camera::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FUNC(Camera::OnMouseScrolled));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNC(Camera::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FUNC(Camera::OnMouseButtonReleased));
	}

	void Camera::UpdateCameraDir(double dx, double dy)
	{
		if (enableCameraMovement)
		{
			yaw += static_cast<float>(dx);
			pitch += static_cast<float>(dy);

			if (pitch > 89.0f)
			{
				pitch = 89.0f;
			}
			else if (pitch < -89.0f)
			{
				pitch = -89.0f;
			}

			UpdateCameraVectors();
		}
	}

	void Camera::UpdateCameraPos(CameraDirection dir, double dt)
	{
		float velocity = (float)dt * speed;

		switch (dir)
		{
		case CameraDirection::FORWARD:
			cameraPos += front * velocity;
			break;
		case CameraDirection::BACKWARDS:
			cameraPos -= front * velocity;
			break;
		case CameraDirection::RIGHT:
			cameraPos += glm::normalize(glm::cross(front, up)) * velocity;
			break;
		case CameraDirection::LEFT:
			cameraPos -= glm::normalize(glm::cross(front, up)) * velocity;
			break;
		}
	}

	void Camera::UpdateCameraZoom(double dy)
	{
		if (zoom >= 1.0f && zoom <= 45.0f)
		{
			zoom -= static_cast<float>(dy);
		}
		else if (zoom < 1.0f)
		{
			zoom = 1.0f;
		}
		else
		{
			zoom = 45.0f;
		}
	}

	void Camera::UpdateCameraSpeed(double dy)
	{
		float mult = dy * 0.2f;
		if (speed >= 0.2f && speed <= 20.0f)
		{
			speed += mult;
		}
		else if (speed < 0.2f)
		{
			speed = 0.2f;
		}
		else
		{
			speed = 20.0f;
		}
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 dir;
		dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		dir.y = sin(glm::radians(pitch));
		dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(dir);

		//right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		//up = glm::normalize(glm::cross(right, front));

		up = glm::vec3(0.0f, 1.0f, 0.0f);
		right = glm::normalize(glm::cross(front, up));
	}

	bool Camera::OnMouseMoved(MouseMovedEvent& e)
	{
		float sens = 1.f;
		
		if (firstMouse)
		{
			lastX = e.GetX();
			lastY = e.GetY();
			firstMouse = false;
		}

		UpdateCameraDir(static_cast<double>(((double)e.GetX() - lastX)) * sens, static_cast<double>((lastY - (double)e.GetY())) * sens);

		lastX = e.GetX();
		lastY = e.GetY();

		return 0;
	}

	bool Camera::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (enableCameraMovement)
		{
			UpdateCameraSpeed(e.GetYOffset());
		}

		return 0;
	}

	bool Camera::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == MouseTags::ButtonRight)
		{
			enableCameraMovement = true;
		}

		return 0;
	}

	bool Camera::OnMouseButtonReleased(MouseButtonReleasedEvent& e)
	{
		if (e.GetMouseButton() == MouseTags::ButtonRight)
		{
			enableCameraMovement = false;
		}

		return 0;
	}
}