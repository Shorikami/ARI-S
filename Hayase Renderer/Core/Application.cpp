#include <hyspch.h>

#include "Application.h"
#include "AppEvent.h"

#include "Empty.h"

#include <glad/glad.h>

namespace Hayase
{
#define BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application(int windowWidth, int windowHeight)
	{
		m_Window = Window::Generate();
		m_Window->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));

		PushLayer(new EmptyScene());
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	void Application::PushLayer(Layer* l)
	{
		m_LayerStack.PushLayer(l);
	}

	void Application::PushOverlay(Layer* l)
	{
		m_LayerStack.PushOverlay(l);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher ed(e);
		ed.Dispatch<WindowCloseEvent>(BIND_EVENT_FUNC(Application::OnWindowClose));

		//std::cout << e.ToString() << std::endl;

		// Handle events on the lower-most layer first
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.m_Handled)
			{
				break;
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Active = false;
		return true;
	}

	void Application::Run()
	{
		while (m_Active)
		{
			glClearColor(1, 1, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			for (Layer* l : m_LayerStack)
			{
				l->Update();
			}

			m_Window->Update();
		}
	}
}