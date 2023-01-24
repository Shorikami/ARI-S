#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>

#include <string>

namespace Hayase
{
	class Window
	{
	public:
		Window(int w, int h, std::string name, int glfwVerMajor = 4, int glfwVerMinor = 0);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		__inline bool ShouldClose() 
		{
			return glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
				&& glfwWindowShouldClose(window) == 0;
		};

		GLFWwindow* GetGLFWwindow() { return window; }

		void SetResizeCallback(void(*func)(GLFWwindow* window, int w, int h));

	private:
		static void FramebufferResizeCallback(GLFWwindow* window, int w, int h);
		void InitWindow(int major, int minor);

		int width;
		int height;
		bool frameBufferResized = false;

		std::string windowName;
		GLFWwindow* window;
	};
}

#endif