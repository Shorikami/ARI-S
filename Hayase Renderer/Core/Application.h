#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"

namespace Hayase
{
	class Application
	{
	public:
		Application(int width = 1280, int height = 720);
		~Application();

		void Run();

	private:
		WindowBase* m_Window;
		bool m_Active = true;
	};
}


#endif