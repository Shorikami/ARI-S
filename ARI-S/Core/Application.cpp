#include <arpch.h>

#include "Application.h"
#include "AppEvent.h"

#include "Tools.h"

#include "../Editor/Editor.h"

#include <glad/glad.h>

namespace ARIS
{
	Application* Application::m_Instance = nullptr;

	Application::Application(int windowWidth, int windowHeight)
	{
		m_Instance = this;

		WindowProperties props{};
		props.s_Width = windowWidth;
		props.s_Height = windowHeight;
		props.s_MajorVer = 4;
		props.s_MinorVer = 5;
		props.s_Title = "ARI-S";

		m_Window = Window::Generate(props);
		m_Window->SetEventCallback(BIND_EVENT_FUNC(Application::OnEvent));

		m_Editor = new Editor();
		PushOverlay(m_Editor);
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
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.m_Handled)
			{
				break;
			}
			(*it)->OnEvent(e);
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
				l->OnUpdate(dt);
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