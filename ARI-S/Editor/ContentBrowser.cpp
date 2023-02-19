#include "arpch.h"
#include "ContentBrowser.h"

#include <imgui.h>

namespace ARIS
{
	// TODO: Change later
	extern const std::filesystem::path s_AssetPath = "Content";

	ContentBrowser::ContentBrowser()
		: m_CurrentDir(s_AssetPath)
	{
		m_DirIcon = Texture("Resources/Icons/ContentBrowser/hoshino.png", GL_LINEAR, GL_REPEAT);
		m_FileIcon = Texture("Resources/Icons/ContentBrowser/c.png", GL_LINEAR, GL_REPEAT);
	}

	void ContentBrowser::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDir != std::filesystem::path(s_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDir = m_CurrentDir.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = (int)(panelWidth / cellSize);
		if (columns < 1)
		{
			columns = 1;
		}

		ImGui::Columns(columns, 0, false);

		for (auto& dirEntry : std::filesystem::directory_iterator(m_CurrentDir))
		{
			
			const auto& path = dirEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			std::string filenameString = relativePath.filename().string();
			ImGui::PushID(filenameString.c_str());

			Texture icon = dirEntry.is_directory() ? m_DirIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon.m_ID, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (dirEntry.is_directory())
				{
					m_CurrentDir /= path.filename();
				}
			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}
		
		ImGui::Columns(1);

		ImGui::End();
	}
}