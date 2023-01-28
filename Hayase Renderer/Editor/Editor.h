#ifndef EDITOR_H
#define EDITOR_H

#include "Layer.h"

namespace Hayase
{
	class Editor : public Layer
	{
	public:
		Editor();
		~Editor();

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();
	private:
	};
}

#endif