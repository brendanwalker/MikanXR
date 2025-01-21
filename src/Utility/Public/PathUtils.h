#pragma once

#include <filesystem>
#include <string>
#include <vector>

//-- utility methods -----
namespace PathUtils
{
	// Add the given directory to the DLL search path
	void addDllSearchDirectory(const std::filesystem::path& dllPath);

	/// Get the location of the owning module path
	std::filesystem::path getModulePath();

	/// Get the location of resource files 
	std::filesystem::path getResourceDirectory();

	/// Make a relative 
	std::filesystem::path makeAbsoluteResourceFilePath(const std::filesystem::path& relative_path);

	/// Get the "home" location where config files can be stored
	std::filesystem::path getHomeDirectory();

	/// Attempts to build a list of all of the files in a directory
	std::vector<std::string> listFilenamesInDirectory(
		const std::filesystem::path& path, 
		const std::string& extension_filter= std::string());
	std::vector<std::string> listDirectoriesInDirectory(
		const std::filesystem::path& path);
	std::vector<std::string> listVolumes();
	
	// Strips the extension from a filename
	std::string removeFileExtension(std::string& filename);

	// Create a unique timestamped filename
	std::filesystem::path makeTimestampedFilePath(
		const std::filesystem::path& parentDir, 
		const std::string& prefix, 
		const std::string& suffix);

	// Create a string from a path but trims it to a max character limit
	// Prefixes the resulting string with "..." if max length is exceeded
	std::string createTrimmedPathString(const std::filesystem::path& path, const size_t maxLength);
};
