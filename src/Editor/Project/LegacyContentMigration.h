#pragma once

#include <filesystem>
#include <string>

// Moves a project written before graph and material kinds had their own
// extensions and folders onto the current layout:
//   - compositor graphs to compositors/*.compgraph, shape graphs to
//     shapes/*.shapegraph, by the class each file records
//   - material graphs to <name>.matgraph beside their material
//   - material folders from shaders/<domain>/<name>/ to
//     compositor_materials/<name>/ or shape_materials/<name>/, the material
//     file renamed to .compmat or .shapemat by its domain
//   - the graph paths stored in the project file, and the material references
//     stored in the compositor and shape graph files
// Runs once per project load, before the project file is read, and touches
// nothing when the project is already current.
namespace LegacyContentMigration
{
// The current stored path for a legacy graph path of the given kind: the kind's
// folder and extension around the file stem. A path that already carries the
// kind's extension is returned unchanged.
std::string migrateCompositorGraphPath(const std::string& storedPath);
std::string migrateShapeGraphPath(const std::string& storedPath);

// The current stored path and asset reference class for a legacy material
// path (shaders/<domain>/<name>/<name>.mat). A path that carries no domain
// folder is returned unchanged with an empty class name.
struct MaterialPathMigration
{
	std::string storedPath;
	std::string assetClassName;
};
MaterialPathMigration migrateMaterialPath(const std::string& storedPath);

// Migrates the project's files and project file in place. Returns true when
// anything changed.
bool migrateProject(const std::filesystem::path& projectDir, const std::filesystem::path& projectFilePath);
} // namespace LegacyContentMigration
