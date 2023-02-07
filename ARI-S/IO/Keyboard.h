#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <GLFW/glfw3.h>

namespace ARIS
{
	class Keyboard
	{
	public:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		static bool Key(int key);
		static bool KeyChanged(int key);
		static bool KeyUp(int key);
		static bool KeyDown(int key);

	private:
		static bool keys[];
		static bool keysChanged[];
	};
}

#endif