#pragma once

#include "MikanUtilityExport.h"

#include <filesystem>
#include <string>
#include <vector>

//-- utility methods -----
namespace PathUtils
{
	// Add the given directory to the DLL search path
	MIKAN_UTILITY_FUNC(void) addDllSearchDirectory(const std::filesystem::path& dllPath);

	/// Get the location of the owning module path
	MIKAN_UTILITY_FUNC(std::filesystem::path) getModulePath();

	/// Get the location of resource files 
	MIKAN_UTILITY_FUNC(std::filesystem::path) getResourceDirectory();

	/// Make a relative 
	MIKAN_UTILITY_FUNC(std::filesystem::path) makeAbsoluteResourceFilePath(const std::filesystem::path& relative_path);

	/// Get the "home" location where config files can be stored
	MIKAN_UTILITY_FUNC(std::filesystem::path) getHomeDirectory();

	/// Attempts to build a list of all of the files in a directory
	MIKAN_UTILITY_FUNC(std::vector<std::string>) listFilenamesInDirectory(
		const std::filesystem::path& path, 
		const std::string& extension_filter= std::string());
	MIKAN_UTILITY_FUNC(std::vector<std::string>) listDirectoriesInDirectory(
		const std::filesystem::path& path);
	MIKAN_UTILITY_FUNC(std::vector<std::string>) listVolumes();
	
	// Strips the extension from a filename
	MIKAN_UTILITY_FUNC(std::string) removeFileExtension(std::string& filename);

	// Create a unique timestamped filename
	MIKAN_UTILITY_FUNC(std::filesystem::path) makeTimestampedFilePath(
		const std::filesystem::path& parentDir, 
		const std::string& prefix, 
		const std::string& suffix);

	// Create a string from a path but trims it to a max character limit
	// Prefixes the resulting string with "..." if max length is exceeded
	MIKAN_UTILITY_FUNC(std::string) createTrimmedPathString(const std::filesystem::path& path, const size_t maxLength);
};
