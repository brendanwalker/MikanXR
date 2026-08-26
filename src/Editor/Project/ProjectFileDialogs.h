#pragma once

#include <filesystem>

// The native file dialogs for picking a project, shared by the main menu screen
// and a project stage's File menu so both spell the filter, the default
// directory, and the new-project naming rule the same way.
namespace ProjectFileDialogs
{
// Returns an empty path when the user cancels
std::filesystem::path pickProjectToOpen();
// Picks a folder and derives "<folder>/<folder name>.mikanproj" from it
std::filesystem::path pickNewProjectPath();
} // namespace ProjectFileDialogs
