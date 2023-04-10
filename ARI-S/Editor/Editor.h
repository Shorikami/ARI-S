#ifndef EDITOR_H
#define EDITOR_H

#include "Layer.h"
#include "FrameMemory.hpp"
#include "Scene.h"

#include "HierarchyPanel.h"
#include "ContentBrowser.h"

#include "Cameras/EditorCamera.h"

#include <glm.hpp>

namespace ARIS
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

		//static void RenderSliderFloat(float w, float h);

	private:
		void CustomizeColors();
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		std::string GetDisplayBuffer() const { return m_DisplayBuffer; }

	private:
		Framebuffer* m_Framebuffer;
		glm::vec2 m_ViewportSize = glm::vec2(0.0f);
		glm::vec2 m_VPBounds[2];

		EditorCamera m_EditorCamera;

		std::shared_ptr<Scene> m_ActiveScene;

		Entity m_HoveredEntity;

		bool m_BlockEvents = true;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		int m_GizmoType = -1;

		HierarchyPanel m_HierarchyPanel;
		ContentBrowser m_ContentBrowser;

		// must be initialized with scene FBO
		std::string m_DisplayBuffer = std::string("SceneFBO");
	};
}

#endif