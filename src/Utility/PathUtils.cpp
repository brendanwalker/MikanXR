// -- includes -----
#include "PathUtils.h"
#include "StringUtils.h"

#include <stdlib.h>
#include <assert.h>

#include <locale>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>

#if defined WIN32 || defined _WIN32 || defined WINCE
#include <windows.h>
#include <direct.h>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': vsnprintf
#define vsnprintf _vsnprintf
#endif
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
	std::string getCurrentDirectory()
	{
		char buff[FILENAME_MAX];

#if defined WIN32 || defined _WIN32 || defined WINCE
		_getcwd(buff, FILENAME_MAX);
#else
		getcwd(buff, FILENAME_MAX);
#endif

		std::string current_working_dir(buff);
		return current_working_dir;
	}

	std::string getResourceDirectory()
	{
		return getCurrentDirectory() + std::string("/resources");
	}

	std::string getHomeDirectory()
	{
		std::string home_dir;

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

	bool createDirectory(const std::string& path)
	{
		bool bSuccess = false;

#if defined WIN32 || defined _WIN32 || defined WINCE
		if (_mkdir(path.c_str()) == 0)
		{
			bSuccess = true;
		}
#else 
		mode_t nMode = 0733; // UNIX style permissions
		if (mkdir(path.c_str(), nMode) == 0)
		{
			bSuccess = true;
		}
#endif
		else if (errno == EEXIST)
		{
			bSuccess = true;
		}

		return bSuccess;
	}

	bool fetchFilenamesInDirectory(std::string path, std::vector<std::string>& out_filenames)
	{
		bool bSuccess = false;

#if defined WIN32 || defined _WIN32 || defined WINCE
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		hFind = FindFirstFileA(path.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			out_filenames.push_back(std::string(FindFileData.cFileName));

			while (FindNextFileA(hFind, &FindFileData))
			{
				out_filenames.push_back(std::string(FindFileData.cFileName));
			}

			FindClose(hFind);

			bSuccess = true;
		}
#else 
		DIR* dir;
		dirent* ent;

		if ((dir = opendir(path.c_str())) != nullptr)
		{
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != nullptr)
			{
				out_filenames.push_back(std::string(ent->d_name));
			}

			closedir(dir);
			bSuccess = true;
		}
#endif
		return bSuccess;
	}

	bool doesFileExist(const std::string& filename)
	{
		std::ifstream file(filename.c_str());

		return (bool)file;
	}

	bool doesDirectoryExist(const std::string& dirPath)
	{
		struct stat info;

		if (stat(dirPath.c_str(), &info) != 0)
			return false;
		else if (info.st_mode & S_IFDIR)  // S_ISDIR() doesn't exist on my windows 
			return true;
		else
			return false;
	}

	std::string baseFileName(const std::string& path, std::string delims)
	{
		return path.substr(path.find_last_of(delims) + 1);
	}

	std::string removeFileExtension(std::string& filename)
	{
		std::string::size_type const p(filename.find_last_of('.'));
		return p > 0 && p != std::string::npos ? filename.substr(0, p) : filename;
	}

	std::string makeTimestampedFilePath(const std::string& parentDir, const std::string& prefix, const std::string& suffix)
	{
		time_t t = time(0);
		struct tm* now = localtime(&t);

		char buffer[128];
		strftime(buffer, 80, "%Y_%m_%d_%H_%M_%S", now);

		char filename[256];
		StringUtils::formatString(filename, 256, "/%s_%s%s", prefix.c_str(), buffer, suffix.c_str());

		std::string result= parentDir + filename;
		return result;
	}
};