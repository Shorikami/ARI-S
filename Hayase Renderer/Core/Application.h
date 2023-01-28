#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Event.h"
#include "LayerStack.h"
#include "../Editor/Editor.h"

#include "Timer.h"

namespace Hayase
{
	class WindowCloseEvent;

	class Application
	{
	public:
		Application(int width = 1280, int height = 720);
		~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* l);
		void PushOverlay(Layer* o);

		inline static Application& Get() { return *m_Instance; }
		inline WindowBase& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		WindowBase* m_Window;
		Editor* m_Editor;

		bool m_Active = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		// singleton
		static Application* m_Instance;
	};
}


#endif