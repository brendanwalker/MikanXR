#pragma once

#include <string>
#include <vector>

//-- utility methods -----
namespace PathUtils
{
	/// Get the current working directory
	std::string getCurrentDirectory();

	/// Get the location of resource files 
	std::string getResourceDirectory();

	/// Get the "home" location where config files can be stored
	std::string getHomeDirectory();

	/// Attempts to create a directory at the given path. Returns 0 on success
	bool createDirectory(const std::string& path);

	/// Attempts to build a list of all of the files in a directory
	bool fetchFilenamesInDirectory(std::string path, std::vector<std::string>& out_filenames);

	/// Returns true if a file exists
	bool doesFileExist(const std::string& filename);

	/// Returns true if a directory exists
	bool doesDirectoryExist(const std::string& dirPath);

	// Extracts the filename from a full file path
	std::string baseFileName(const std::string& path, std::string delims = "/\\");

	// Strips the extension from a filename
	std::string removeFileExtension(std::string& filename);

	// Create a unique timestamped filename
	std::string makeTimestampedFilePath(const std::string& parentDir, const std::string& prefix, const std::string& suffix);
};
