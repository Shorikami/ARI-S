#ifndef FILEDIALOGS_H
#define FILEDIALOGS_H

#include <string>

namespace ARIS
{
	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};
}

#endif