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
		auto currTime = std::chrono::high_resolution_clock::now();

		do
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currTime).count();
			currTime = newTime;
		} 
		while (window.ShouldClose());
	
	}
}