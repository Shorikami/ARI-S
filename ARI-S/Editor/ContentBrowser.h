#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <filesystem>

namespace ARIS
{
	class ContentBrowser
	{
	public:
		ContentBrowser();

		void OnImGuiRender();

	private:
		std::filesystem::path m_CurrentDir;

	};
}

#endif