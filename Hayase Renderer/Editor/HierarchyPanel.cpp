#include <hyspch.h>

#include "HierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <gtc/type_ptr.hpp>

#include <cstring>

namespace Hayase
{
	template<typename T, typename Function>
	static void DrawComponent(const std::string& name, Entity e, Function func)
	{
		const ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | 
			ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (e.HasComponent<T>())
		{
			auto& comp = e.GetComponent<T>();
			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
				{
					removeComponent = true;
				}
				ImGui::EndPopup();
			}

			if (open)
			{
				func(comp);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				e.RemoveComponent<T>();
			}
				
		}
	}

	HierarchyPanel::HierarchyPanel(const std::shared_ptr<Scene>& scene)
	{
		SetContext(scene);
	}

	void HierarchyPanel::SetContext(const std::shared_ptr<Scene>& scene)
	{
		m_Context = scene;
		m_SelectionContext = {};
	}

	void HierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			m_Context->m_Registry.each([&](auto entityID)
				{
					Entity e{ entityID, m_Context.get() };
					DrawEntityNode(e);
				});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				m_SelectionContext = {};
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					m_Context->CreateEntity("Empty Entity");

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}

		ImGui::End();
	}

	void HierarchyPanel::SetSelectedEntity(Entity e)
	{
		m_SelectionContext = e;
	}

	template <typename T>
	void HierarchyPanel::DisplayAddComponentEntry(const std::string& name)
	{
		if (!m_SelectionContext.HasComponent<T>())
		{
			m_SelectionContext.AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}

	void HierarchyPanel::DrawEntityNode(Entity e)
	{
		auto& tag = e.GetComponent<TagComponent>().s_Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == e) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)e, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = e;
		}

		bool deleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				deleted = true;
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
			{
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (deleted)
		{
			m_Context->DestroyEntity(e);
			if (m_SelectionContext == e)
			{
				m_SelectionContext = {};
			}
		}
	}

	void HierarchyPanel::DrawComponents(Entity e)
	{
		if (e.HasComponent<TagComponent>())
		{
			auto& tag = e.GetComponent<TagComponent>().s_Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine(); ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComp");
		}

		if (ImGui::BeginPopup("AddComp"))
		{
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
	}
}