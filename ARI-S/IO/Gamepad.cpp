#include <arpch.h>

#include "Gamepad.h"

namespace ARIS
{
	Gamepad::Gamepad(int i)
	{
		id = GetID(i);

		Update();
	}

	void Gamepad::Update()
	{
		present = glfwJoystickPresent(id);
		if (present)
		{
			name = glfwGetJoystickName(id);
			axes = glfwGetJoystickAxes(id, &axesCount);
			buttons = glfwGetJoystickButtons(id, &buttonCount);
		}
	}

	float Gamepad::AxesState(int axis)
	{
		if (present)
		{
			return axes[axis];
		}

		return 0.0f;
	}

	unsigned char Gamepad::ButtonState(int b)
	{
		return present ? buttons[b] : GLFW_RELEASE;
	}

	int Gamepad::GetAxesCount()
	{
		return axesCount;
	}

	int Gamepad::GetButtonCount()
	{
		return buttonCount;
	}

	bool Gamepad::IsPresent()
	{
		return present;
	}

	const char* Gamepad::GetName()
	{
		return name;
	}

	int Gamepad::GetID(int i)
	{
		return GLFW_JOYSTICK_1 + i;
	}
}