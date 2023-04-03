#include <arpch.h>

#include "HierarchyPanel.h"
#include "ModelBuilder.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <gtc/type_ptr.hpp>

#include <cstring>

namespace ARIS
{
	extern const std::filesystem::path s_AssetPath;

#pragma region staticFuncs
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
			if (std::is_same<T, TransformComponent>::value)
			{
				ImGui::Dummy(ImVec2{ lineHeight, lineHeight });
			}
			else
			{
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
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

	static int TextResizeCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			ImVector<char>* my_str = (ImVector<char>*)data->UserData;
			IM_ASSERT(my_str->begin() == data->Buf);
			my_str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
			data->Buf = my_str->begin();
		}
		return 0;
	}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}
#pragma endregion staticFuncs

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
				}

				ImGui::EndPopup();
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
			if (ImGui::MenuItem(name.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
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
			//ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			//bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			//if (opened)
			//{
			//	ImGui::TreePop();
			//}
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
			DisplayAddComponentEntry<MeshComponent>("Model");
			DisplayAddComponentEntry<PointLightComponent>("Point Light");
			DisplayAddComponentEntry<DirectionLightComponent>("Directional Light");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", e, [](auto& comp)
		{
			DrawVec3Control("Translation", comp.m_Translation);
			glm::vec3 rot = glm::degrees(comp.m_Rotation);
			DrawVec3Control("Rotation", rot);
			comp.m_Rotation = glm::radians(rot);
			DrawVec3Control("Scale", comp.m_Scale, 1.0f);
		});

		DrawComponent<MeshComponent>("Model", e, [](auto& comp)
		{
			//Model* currItem = &comp.m_Model;
			//if (ImGui::BeginCombo("##custom combo", currItem->GetName().c_str()))
			//{
			//	std::unordered_map<std::string, Model*> modelTable = ModelBuilder::Get().GetModelTable();
			//	for (std::unordered_map<std::string, Model*>::iterator it =
			//		modelTable.begin(); it != modelTable.end(); ++it)
			//	{
			//		Model* item = it->second;
			//		bool isSelected = (currItem == item);
			//		if (ImGui::Selectable(item->GetName().c_str(), isSelected))
			//		{
			//			currItem = item;
			//			comp.m_Model = *currItem;
			//		}
			//		if (isSelected)
			//		{
			//			ImGui::SetItemDefaultFocus();
			//		}
			//	}
			//	ImGui::EndCombo();
			//}

			ImGui::Button("Diffuse Texture", ImVec2(128.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texPath = std::filesystem::path(s_AssetPath) / path;
					Texture* tex = new Texture(texPath.string(), GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE);

					if (tex->m_IsLoaded)
					{
						for (Mesh m : comp.m_Model.GetMeshes())
						{
							m.GetTextures().push_back(*tex);
						}
						comp.m_DiffuseTex = tex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			//if (comp.m_DiffuseTex)
			//{
			//	ImGui::SameLine(); ImGui::Image((void*)(intptr_t)comp.m_DiffuseTex->m_ID, ImVec2{ 128, 128 },
			//		ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//}

			ImGui::Button("Normal Texture", ImVec2(128.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texPath = std::filesystem::path(s_AssetPath) / path;
					Texture* tex = new Texture(texPath.string(), GL_LINEAR, GL_REPEAT, false, aiTextureType_NORMALS);

					if (tex->m_IsLoaded)
					{
						for (Mesh m : comp.m_Model.GetMeshes())
						{
							m.GetTextures().push_back(*tex);
						}
						comp.m_NormalTex = tex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			//if (comp.m_NormalTex)
			//{
			//	ImGui::SameLine(); ImGui::Image((void*)(intptr_t)comp.m_NormalTex->m_ID, ImVec2{ 128, 128 },
			//		ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//}

			ImGui::Button("Metallic Texture", ImVec2(128.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texPath = std::filesystem::path(s_AssetPath) / path;
					Texture* tex = new Texture(texPath.string(), GL_LINEAR, GL_REPEAT, false, aiTextureType_METALNESS);

					if (tex->m_IsLoaded)
					{
						for (Mesh m : comp.m_Model.GetMeshes())
						{
							m.GetTextures().push_back(*tex);
						}
						comp.m_Metallic = tex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			//if (comp.m_Metallic)
			//{
			//	ImGui::SameLine(); ImGui::Image((void*)(intptr_t)comp.m_Metallic->m_ID, ImVec2{ 128, 128 },
			//		ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//}


			ImGui::Button("Roughness Texture", ImVec2(128.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texPath = std::filesystem::path(s_AssetPath) / path;
					Texture* tex = new Texture(texPath.string(), GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE_ROUGHNESS);

					if (tex->m_IsLoaded)
					{
						for (Mesh m : comp.m_Model.GetMeshes())
						{
							m.GetTextures().push_back(*tex);
						}
						comp.m_Roughness = tex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			//if (comp.m_Roughness)
			//{
			//	ImGui::SameLine(); ImGui::Image((void*)(intptr_t)comp.m_Roughness->m_ID, ImVec2{ 128, 128 },
			//		ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//}

			ImGui::Button("Metallic/Roughness Texture", ImVec2(128.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texPath = std::filesystem::path(s_AssetPath) / path;
					Texture* tex = new Texture(texPath.string(), GL_LINEAR, GL_REPEAT, false, aiTextureType_UNKNOWN);

					if (tex->m_IsLoaded)
					{
						for (Mesh m : comp.m_Model.GetMeshes())
						{
							m.GetTextures().push_back(*tex);
						}
						comp.m_MetalRough = tex;
					}
				}
				ImGui::EndDragDropTarget();
			}

			//if (comp.m_MetalRough)
			//{
			//	ImGui::SameLine(); ImGui::Image((void*)(intptr_t)comp.m_MetalRough->m_ID, ImVec2{ 128, 128 },
			//		ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			//}

			std::string vBuf = comp.m_VertexSrc, fBuf = comp.m_FragmentSrc;
			if (ImGui::InputText("Vertex Shader Path", &vBuf, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				comp.m_VertexSrc = std::string(vBuf);
			}

			if (ImGui::InputText("Fragment Shader Path", &fBuf, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				comp.m_FragmentSrc = std::string(fBuf);
			}

			if (ImGui::Button("Reload Model Shader"))
			{
				comp.ReloadShader();
			}
		});

		DrawComponent<PointLightComponent>("Point Light", e, [](auto& comp)
		{
			ImGui::SliderFloat("Range", &comp.m_Range, 0.0f, 100.0f);
			ImGui::SliderFloat("Intensity", &comp.m_Intensity, 0.0f, 10.0f);
			ImGui::ColorPicker4("Color", &comp.m_Color[0]);
		});

		DrawComponent<DirectionLightComponent>("Directional Light", e, [](auto& comp)
		{
			ImGui::Checkbox("Use Perspective", &comp.m_UsePerspective);
			ImGui::Separator();

			ImGui::SliderFloat("Width", &comp.m_Width, 0.0f, 100.0f);
			ImGui::SliderFloat("Height", &comp.m_Height, 0.0f, 100.0f);
			ImGui::SliderFloat("Near Clip", &comp.m_Near, 0.1f, comp.m_Far);
			ImGui::SliderFloat("Far Clip", &comp.m_Far, 0.1f, 100.0f);
		});


	}
}