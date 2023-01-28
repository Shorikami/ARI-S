#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Event.h"
#include "LayerStack.h"

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

	private:
		bool OnWindowClose(WindowCloseEvent& e);

		WindowBase* m_Window;
		bool m_Active = true;
		LayerStack m_LayerStack;
	};
}


#endif