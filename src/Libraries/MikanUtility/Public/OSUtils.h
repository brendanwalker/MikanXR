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
};
