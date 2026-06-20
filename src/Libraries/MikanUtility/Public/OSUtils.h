#pragma once

#include "MikanUtilityExport.h"

#include <filesystem>
#include <string>

//-- utility methods -----
namespace OSUtils
{
/// Open a file with the default system application
/// On Windows: uses ShellExecute
/// On macOS/Linux: can be extended to use open/xdg-open
MIKAN_UTILITY_FUNC(bool) openFileWithDefaultApplication(const std::filesystem::path& filePath);

/// Open a file with a specific editor command (e.g. "code --reuse-window").
/// The file path is appended as a quoted argument after the command.
/// Falls back to openFileWithDefaultApplication when editorCommand is empty.
MIKAN_UTILITY_FUNC(bool) openFileWithApplication(
	const std::filesystem::path& filePath,
	const std::string& editorCommand);
}; // namespace OSUtils
