#include <hyspch.h>

#include "InputPoll.h"
#include "Application.h"
#include <GLFW/glfw3.h>

namespace Hayase
{
	bool InputPoll::IsKeyPressed(const KeyCode key)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		return glfwGetKey(window, static_cast<int32_t>(key)) == GLFW_PRESS;
	}

	bool InputPoll::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		return glfwGetMouseButton(window, static_cast<int32_t>(button)) == GLFW_PRESS;
	}

	glm::vec2 InputPoll::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return glm::vec2(xPos, yPos);
	}
}