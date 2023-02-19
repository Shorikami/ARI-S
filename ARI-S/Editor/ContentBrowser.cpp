#include "arpch.h"
#include "ContentBrowser.h"

#include <imgui.h>

namespace ARIS
{
	// TODO: Change later
	static const std::filesystem::path s_AssetPath = "Content";

	ContentBrowser::ContentBrowser()
		: m_CurrentDir(s_AssetPath)
	{
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

		for (auto& dirEntry : std::filesystem::directory_iterator(m_CurrentDir))
		{
			const auto& path = dirEntry.path();
			auto relativePath = std::filesystem::relative(dirEntry.path(), s_AssetPath);
			std::string filenameString = relativePath.filename().string();

			if (dirEntry.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDir /= path.filename();
				}
			}

			else
			{
				if (ImGui::Button(filenameString.c_str()))
				{

				}
			}
		}
		

		ImGui::End();
	}
}