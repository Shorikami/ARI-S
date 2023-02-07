#ifndef EDITOR_H
#define EDITOR_H

#include "Layer.h"
#include "FrameMemory.hpp"
#include "Scene.h"

#include "HierarchyPanel.h"

#include <glm.hpp>

namespace Hayase
{
	class Editor : public Layer
	{
	public:
		Editor();
		~Editor();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(DeltaTime dt) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool b) { m_BlockEvents = b; }

	private:
		void CustomizeColors();
		void NewScene();
		void OpenScene();
		void SaveSceneAs();

	private:
		Framebuffer* m_Framebuffer;
		glm::vec2 m_ViewportSize = glm::vec2(0.0f);
		std::shared_ptr<Scene> m_ActiveScene;

		bool m_BlockEvents = true;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		HierarchyPanel m_HierarchyPanel;
	};
}

#endif