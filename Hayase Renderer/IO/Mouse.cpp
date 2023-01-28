#include <hyspch.h>

#include "Mouse.h"

namespace Hayase
{
	double Mouse::x = 0;
	double Mouse::y = 0;

	double Mouse::lastX = 0;
	double Mouse::lastY = 0;

	double Mouse::dx = 0;
	double Mouse::dy = 0;

	double Mouse::scrollDX = 0;
	double Mouse::scrollDY = 0;

	bool Mouse::firstMouse = true;

	bool Mouse::buttons[GLFW_MOUSE_BUTTON_LAST] = { 0 };
	bool Mouse::buttonsChanged[GLFW_MOUSE_BUTTON_LAST] = { 0 };


	void Mouse::CursorPosCallback(GLFWwindow* window, double x_, double y_)
	{
		x = x_;
		y = y_;

		if (firstMouse)
		{
			lastX = x;
			lastY = y;
			firstMouse = false;
		}

		dx = x - lastX;
		dy = lastY - y;
		lastX = x;
		lastY = y;
	}

	void Mouse::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action != GLFW_RELEASE)
		{
			if (!buttons[button])
			{
				buttons[button] = true;
			}
		}
		else
		{
			buttons[button] = false;
		}

		buttonsChanged[button] = action != GLFW_REPEAT;
	}

	void Mouse::MouseWheelCallback(GLFWwindow* window, double dx, double dy)
	{
		scrollDX = dx;
		scrollDY = dy;
	}


	double Mouse::GetMouseX()
	{
		return x;
	}

	double Mouse::GetMouseY()
	{
		return y;
	}

	double Mouse::GetDX()
	{
		double dx_ = dx;
		dx = 0;
		return dx_;
	}

	double Mouse::GetDY()
	{
		double dy_ = dy;
		dy = 0;
		return dy_;
	}

	double Mouse::GetScrollDX()
	{
		double dx = scrollDX;
		scrollDX = 0;
		return dx;
	}

	double Mouse::GetScrollDY()
	{
		double dy = scrollDY;
		scrollDY = 0;
		return dy;
	}

	bool Mouse::Button(int button)
	{
		return buttons[button];
	}

	bool Mouse::ButtonChanged(int button)
	{
		bool res = buttonsChanged[button];
		buttonsChanged[button] = false;
		return res;
	}

	bool Mouse::ButtonUp(int button)
	{
		return !buttons[button] && ButtonChanged(button);
	}

	bool Mouse::ButtonDown(int button)
	{
		return buttons[button] && ButtonChanged(button);
	}

}