// -- includes -----
#include "OSUtils.h"

#if defined WIN32 || defined _WIN32 || defined WINCE
#include <windows.h>
#include <shellapi.h>
#else
#include <cstdlib>
#endif

// -- public methods -----
namespace OSUtils
{
	bool openFileWithDefaultApplication(const std::filesystem::path& filePath)
	{
#if defined WIN32 || defined _WIN32 || defined WINCE
		// Use ShellExecute on Windows
		HINSTANCE result = ShellExecuteA(
			NULL,
			"open",
			filePath.string().c_str(),
			NULL,
			NULL,
			SW_SHOWNORMAL);

		// ShellExecute returns a value greater than 32 on success
		return reinterpret_cast<intptr_t>(result) > 32;
#elif defined(__APPLE__)
		// Use 'open' command on macOS
		std::string command = "open \"" + filePath.string() + "\"";
		return system(command.c_str()) == 0;
#else
		// Use 'xdg-open' command on Linux
		std::string command = "xdg-open \"" + filePath.string() + "\"";
		return system(command.c_str()) == 0;
#endif
	}
};
