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
	
		scene = new Default(WindowInfo::windowWidth, WindowInfo::windowHeight);
		scene->Init();

		auto currTime = std::chrono::high_resolution_clock::now();

		do
		{
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currTime).count();
			currTime = newTime;

			scene->ProcessInput(window.GetGLFWwindow(), frameTime);
			scene->Display();


			glfwSwapBuffers(window.GetGLFWwindow());
			glfwPollEvents();
		} 
		while (window.ShouldClose());
		
		glfwTerminate();
	}
}