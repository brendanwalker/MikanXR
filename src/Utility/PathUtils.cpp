// -- includes -----
#include "PathUtils.h"
#include "StringUtils.h"

#include <assert.h>
#include <filesystem>

#if defined WIN32 || defined _WIN32 || defined WINCE
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <time.h>
#include <unistd.h>
#endif

// -- public methods -----
namespace PathUtils
{
	std::filesystem::path getResourceDirectory()
	{
		std::filesystem::path resourceDir= std::filesystem::current_path();
		resourceDir/= std::string("resources");

		return resourceDir;
	}

	std::filesystem::path makeAbsoluteResourceFilePath(const std::filesystem::path& relative_path)
	{
		if (relative_path.is_absolute())
			return relative_path;

		std::filesystem::path full_path(getResourceDirectory());
		full_path /= relative_path;

		return full_path;
	}

	std::filesystem::path getHomeDirectory()
	{
		std::filesystem::path home_dir;

#if defined WIN32 || defined _WIN32 || defined WINCE
		size_t homedir_buffer_req_size;
		char homedir_buffer[512];
		getenv_s(&homedir_buffer_req_size, homedir_buffer, "APPDATA");
		assert(homedir_buffer_req_size <= sizeof(homedir_buffer));
		home_dir = homedir_buffer;
#else    
		// if run as root, use system-wide data directory
		if (geteuid() == 0)
		{
			home_dir = "/etc/";
		}
		else
		{
			homedir = getenv("HOME");
		}
#endif
		return home_dir;
	}

	std::vector<std::string> listFilenamesInDirectory(
		const std::filesystem::path& path, 
		const std::string& extension_filter)
	{
		std::vector<std::string> filenames;

		for (auto const& dir_entry : std::filesystem::directory_iterator{path})
		{
			if (dir_entry.is_regular_file())
			{
				const std::filesystem::path& filename= dir_entry.path().filename();

				if (extension_filter.empty() || filename.extension() == extension_filter)
				{
					filenames.push_back(filename.string());
				}
			}
		}

		return filenames;
	}

	std::vector<std::string> listDirectoriesInDirectory(
		const std::filesystem::path& path)
	{
		std::vector<std::string> dirnames;

		for (auto const& dir_entry : std::filesystem::directory_iterator{path})
		{
			if (dir_entry.is_directory())
			{
				const std::filesystem::path& filename = dir_entry.path().filename();

				dirnames.push_back(filename.string());
			}
		}

		return dirnames;
	}

	std::vector<std::string> listVolumes()
	{
		std::vector<std::string> result;

		#if defined WIN32 || defined _WIN32 || defined WINCE
		char szLogicalDrives[MAX_PATH] = {0};
		DWORD dwResult = GetLogicalDriveStringsA(MAX_PATH, szLogicalDrives);
		if (dwResult > 0 && dwResult <= MAX_PATH)
		{
			char* szSingleDrive = szLogicalDrives;
			while (*szSingleDrive)
			{
				result.push_back(szSingleDrive);

				// get the next drive
				szSingleDrive += strlen(szSingleDrive) + 1;
			}
		}
		#else
		//TODO: Just list the root directory for now unix systems
		result.push_back("/");
		#endif

		return result;
	}

	std::filesystem::path makeTimestampedFilePath(
		const std::filesystem::path& parentDir, 
		const std::string& prefix, 
		const std::string& suffix)
	{
		time_t t = time(0);
		struct tm* now = localtime(&t);

		char buffer[128];
		strftime(buffer, 80, "%Y_%m_%d_%H_%M_%S", now);

		char filename[256];
		StringUtils::formatString(filename, 256, "%s_%s%s", prefix.c_str(), buffer, suffix.c_str());

		std::filesystem::path result= parentDir;
		result/= filename;

		return result;
	}
};