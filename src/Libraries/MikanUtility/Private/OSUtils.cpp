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
bool openFileWithApplication(
	const std::filesystem::path& filePath,
	const std::string& editorCommand)
{
	if (editorCommand.empty())
		return openFileWithDefaultApplication(filePath);

	// Split "exe [args]" on the first space so ShellExecute gets exe and
	// parameters separately (avoids a console window on Windows).
	const size_t spacePos= editorCommand.find(' ');
	const std::string exe= editorCommand.substr(0, spacePos);
	const std::string existingArgs=
		(spacePos != std::string::npos) ? editorCommand.substr(spacePos + 1) + " " : "";
	const std::string quotedPath= "\"" + filePath.generic_string() + "\"";
	const std::string fullArgs= existingArgs + quotedPath;

#if defined WIN32 || defined _WIN32 || defined WINCE
	HINSTANCE result= ShellExecuteA(
		NULL, "open", exe.c_str(), fullArgs.c_str(), NULL, SW_SHOWNORMAL);
	return reinterpret_cast<intptr_t>(result) > 32;
#else
	const std::string cmd= editorCommand + " " + quotedPath;
	return system(cmd.c_str()) == 0;
#endif
}

bool openFileWithDefaultApplication(const std::filesystem::path& filePath)
{
#if defined WIN32 || defined _WIN32 || defined WINCE
	// Use ShellExecute on Windows
	HINSTANCE result= ShellExecuteA(
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
	std::string command= "open \"" + filePath.string() + "\"";
	return system(command.c_str()) == 0;
#else
	// Use 'xdg-open' command on Linux
	std::string command= "xdg-open \"" + filePath.string() + "\"";
	return system(command.c_str()) == 0;
#endif
}
}; // namespace OSUtils
