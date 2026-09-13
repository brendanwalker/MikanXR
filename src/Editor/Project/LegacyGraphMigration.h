#pragma once

#include <filesystem>
#include <string>

// Moves a project written before graph kinds had their own extensions and
// folders onto the current layout: compositor graphs to compositors/*.compgraph,
// shape graphs to shapes/*.shapegraph, material graphs to <name>.matgraph beside
// their .mat, and the stored graph paths in the project file to match. Runs once
// per project load, before the project file is read, and touches nothing when
// the project is already current.
namespace LegacyGraphMigration
{
// The current stored path for a legacy graph path of the given kind: the kind's
// folder and extension around the file stem. A path that already carries the
// kind's extension is returned unchanged.
std::string migrateCompositorGraphPath(const std::string& storedPath);
std::string migrateShapeGraphPath(const std::string& storedPath);

// Migrates the project's files and project file in place. Returns true when
// anything changed.
bool migrateProject(const std::filesystem::path& projectDir, const std::filesystem::path& projectFilePath);
} // namespace LegacyGraphMigration
