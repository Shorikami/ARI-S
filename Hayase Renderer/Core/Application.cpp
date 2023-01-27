#include "Application.h"
#include <glad/glad.h>

namespace Hayase
{
	Application::Application(int windowWidth, int windowHeight)
	{
		m_Window = Window::Generate();
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	void Application::Run()
	{
		while (m_Active)
		{
			glClearColor(1, 1, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->Update();
		}
	}
}