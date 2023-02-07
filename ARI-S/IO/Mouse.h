#ifndef MOUSE_H
#define MOUSE_H

#include <GLFW/glfw3.h>

namespace ARIS
{
	class Mouse
	{
	public:
		static void CursorPosCallback(GLFWwindow* window, double x_, double y_);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void MouseWheelCallback(GLFWwindow* window, double dx, double dy);

		static double GetMouseX();
		static double GetMouseY();

		static double GetDX();
		static double GetDY();

		static double GetScrollDX();
		static double GetScrollDY();

		static bool Button(int button);
		static bool ButtonChanged(int button);
		static bool ButtonUp(int button);
		static bool ButtonDown(int button);

	private:
		static double x, y;

		static double lastX, lastY;

		static double dx, dy;

		static double scrollDX, scrollDY;

		static bool firstMouse;

		static bool buttons[];
		static bool buttonsChanged[];
	};
}

#endif