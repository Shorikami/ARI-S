#ifndef APPLICATION_H
#define APPLICATION_H

#include "../Scene Management/SceneFramework.h"
#include "../Scene Management/Scenes/Deferred.h"
#include "../Scene Management/Scenes/DepthBuffer.h"

#include <memory>
#include <vector>

namespace Hayase
{
	class Application
	{
	public:
		Application(int width = 1280, int height = 720);
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

		void Run();

	private:
		Scene* activeScene;
	};
}


#endif