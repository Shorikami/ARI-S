#include "Application.h"

#include <chrono>

namespace Hayase
{
	Application::Application(int windowWidth, int windowHeight)
		: window(windowWidth, windowHeight, "Hayase Renderer", 4, 0)
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		// VSync
		//glfwSwapInterval(1);

		scene = new Deferred(WindowInfo::windowWidth, WindowInfo::windowHeight);
		scene->Init();

		//auto currTime = std::chrono::high_resolution_clock::now();
		unsigned int counter = 0;
		float fps = 0.0f;
		float currTime = 0.0f, prevTime = 0.0f, frameTime = 0.0f;

		do
		{
			//auto newTime = std::chrono::high_resolution_clock::now();
			currTime = glfwGetTime();
			frameTime = currTime - prevTime;
			prevTime = currTime;

			scene->ProcessInput(window.GetGLFWwindow(), frameTime);
			scene->Display(1.0f / frameTime);

			glfwSwapBuffers(window.GetGLFWwindow());
			glfwPollEvents();
		} 
		while (window.ShouldClose());
		
		glfwTerminate();
	}
}