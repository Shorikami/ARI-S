#include <hyspch.h>

#include "Timer.h"

#include <GLFW/glfw3.h>

namespace Hayase
{
	float Time::GetTime()
	{
		return static_cast<float>(glfwGetTime());
	}
}