#include "ProjectFileDialogs.h"
#include "LocText.h"
#include "PathUtils.h"
#include "ProjectManager.h"

#include "tinyfiledialogs.h"

namespace ProjectFileDialogs
{
std::filesystem::path pickProjectToOpen()
{
	const std::string defaultPath= (PathUtils::getProjectsRootDirectory() / "").string();
	static const char* filterItems[1]= {"*.mikanproj"};

	const char* picked= tinyfd_openFileDialog(locText("mainMenu.openProjectDialogTitle"), defaultPath.c_str(), 1,
											  filterItems, locText("mainMenu.projectFilesFilter"), 1);

	if (picked == nullptr || picked[0] == '\0')
		return std::filesystem::path();

	return std::filesystem::path(picked);
}

std::filesystem::path pickNewProjectPath()
{
	const std::string defaultPath= (PathUtils::getProjectsRootDirectory() / "").string();

	const char* picked= tinyfd_selectFolderDialog(locText("mainMenu.newProjectFolderDialogTitle"), defaultPath.c_str());

	if (picked == nullptr || picked[0] == '\0')
		return std::filesystem::path();

	// A project is named after the folder that holds it
	const std::filesystem::path projectFolderPath(picked);
	const std::string projectFileName=
		projectFolderPath.filename().string() + std::string(ProjectManager::k_mikanProjectFileExtension);

	return projectFolderPath / projectFileName;
}
} // namespace ProjectFileDialogs
