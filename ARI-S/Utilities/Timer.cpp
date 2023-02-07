#include <arpch.h>

#include "Timer.h"

#include <GLFW/glfw3.h>

namespace ARIS
{
	float Time::GetTime()
	{
		return static_cast<float>(glfwGetTime());
	}
}