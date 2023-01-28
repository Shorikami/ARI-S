#include <hyspch.h>

#include "Application.h"
#include "AppEvent.h"

#include "Empty.h"
#include "Deferred.h"

#include "Tools.h"

#include "../Editor/Editor.h"

#include <glad/glad.h>

namespace Hayase
{
	Application* Application::m_Instance = nullptr;

	Application::Application(int windowWidth, int windowHeight)
	{
		m_Instance = this;

		WindowProperties props{};
		props.s_Width = windowWidth;
		props.s_Height = windowHeight;
		props.s_MajorVer = 4;
		props.s_MinorVer = 3;
		props.s_Title = "Hayase Renderer";

		m_Window = Window::Generate(props);
		m_Window->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));

		m_Editor = new Editor();
		PushOverlay(m_Editor);

		PushLayer(new Deferred(windowWidth, windowHeight));
	}

	Application::~Application()
	{
		glfwTerminate();
	}

	void Application::PushLayer(Layer* l)
	{
		m_LayerStack.PushLayer(l);
		l->OnAttach();
	}

	void Application::PushOverlay(Layer* l)
	{
		m_LayerStack.PushOverlay(l);
		l->OnAttach();
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
			float time = Time::GetTime();
			DeltaTime dt = time - m_LastFrameTime;
			m_LastFrameTime = time;
			
			for (Layer* l : m_LayerStack)
			{
				l->Update(dt);
			}

			m_Editor->Begin();
			for (Layer* l : m_LayerStack)
			{
				l->OnImGuiRender();
			}
			m_Editor->End();
			
			m_Window->Update();
		}
	}

	void Application::Close()
	{
		m_Active = false;
	}
}