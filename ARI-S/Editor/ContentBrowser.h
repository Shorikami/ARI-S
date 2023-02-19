#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <filesystem>

#include "Texture.h"

namespace ARIS
{
	class ContentBrowser
	{
	public:
		ContentBrowser();

		void OnImGuiRender();

	private:
		std::filesystem::path m_BaseDir;
		std::filesystem::path m_CurrentDir;

		Texture m_DirIcon, m_FileIcon;
	};
}

#endif