#ifndef HIERARCHYPANEL_H
#define HIERARCHYPANEL_H

#include "SceneFramework.h"
#include "Entity.h"

namespace Hayase
{
	class HierarchyPanel
	{
	public:
		HierarchyPanel() = default;
		HierarchyPanel(const std::shared_ptr<Scene>& scene);

		void SetContext(const std::shared_ptr<Scene>& scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_SelectionContext; }
		void SetSelectedEntity(Entity e);

	private:
		template <typename T>
		void DisplayAddComponentEntry(const std::string& name);

		void DrawEntityNode(Entity e);
		void DrawComponents(Entity e);

	private:
		std::shared_ptr<Scene> m_Context;
		Entity m_SelectionContext;
	};
}

#endif