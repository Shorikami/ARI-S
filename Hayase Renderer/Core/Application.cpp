#include "Application.h"

#include <chrono>

namespace Hayase
{
	Application::Application(int windowWidth, int windowHeight)
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		// VSync
		//glfwSwapInterval(1);

		activeScene = new Deferred(WindowInfo::windowWidth, WindowInfo::windowHeight);
		activeScene->Init();

		unsigned int counter = 0;
		float fps = 0.0f;
		float currTime = 0.0f, prevTime = 0.0f, frameTime = 0.0f;

		do
		{
			currTime = glfwGetTime();
			frameTime = currTime - prevTime;
			prevTime = currTime;

			activeScene->ProcessInput(activeScene->window.GetGLFWwindow(), frameTime);
			activeScene->Display(1.0f / frameTime);

			glfwSwapBuffers(activeScene->window.GetGLFWwindow());
			glfwPollEvents();
		} 
		while (activeScene->window.ShouldClose());
		
		glfwTerminate();
	}
}