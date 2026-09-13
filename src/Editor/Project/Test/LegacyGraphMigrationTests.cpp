#include "LegacyGraphMigrationTests.h"
#include "unit_test.h"

#include "LegacyGraphMigration.h"
#include "PathUtils.h"

#include <configuru.hpp>

#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <string>

namespace
{
struct MigrationTestProject
{
	std::filesystem::path root;
	std::filesystem::path projectDir;
	std::filesystem::path projectFilePath;
	std::filesystem::path savedProjectDir;

	MigrationTestProject()
	{
		savedProjectDir= PathUtils::getProjectDirectory();

		root= std::filesystem::temp_directory_path() / "mikan_legacy_graph_migration_tests";
		std::filesystem::remove_all(root);
		projectDir= root / "project";
		projectFilePath= projectDir / "project.mikanproj";

		std::filesystem::create_directories(projectDir / "graphs" / "shapes");
		std::filesystem::create_directories(projectDir / "shaders" / "compositor" / "m");

		writeFile(projectDir / "graphs" / "comp.graph", R"({ "class_name": "CompositorNodeGraph", "nodes": [] })");
		writeFile(projectDir / "graphs" / "shapes" / "quad.graph",
				  R"({ "class_name": "ShapeNodeGraph", "nodes": [] })");
		writeFile(projectDir / "shaders" / "compositor" / "m" / "m.graph",
				  R"({ "class_name": "MaterialNodeGraph", "nodes": [] })");
		writeFile(
			projectDir / "shaders" / "compositor" / "m" / "m.mat",
			R"({ "materialName": "m", "vertexAttributes": [], "uniformSemanticMap": {}, "sourceGraphPath": "m.graph" })");

		// Two compositors (one naming a bundled graph that was never in the project)
		// and one shape, nested the way the object system definitions nest them
		writeFile(projectFilePath, R"({
	"CompositorObjectSystem": {
		"components": [
			{ "component_id": 1, "compositor_graph_path": { "class_name": "NodeGraphAssetReference", "asset_path": "graphs/comp.graph" } },
			{ "component_id": 2, "compositor_graph_path": { "class_name": "NodeGraphAssetReference", "asset_path": "graphs/color_key_graph.graph" } }
		]
	},
	"QuadShapeSystem": {
		"components": [
			{ "component_id": 3, "shape_graph_path": { "class_name": "NodeGraphAssetReference", "asset_path": "graphs/shapes/quad.graph" } }
		]
	}
})");

		PathUtils::setProjectDirectory(projectDir);
	}

	~MigrationTestProject()
	{
		PathUtils::setProjectDirectory(savedProjectDir);
		std::error_code ec;
		std::filesystem::remove_all(root, ec);
	}

	static void writeFile(const std::filesystem::path& path, const std::string& content)
	{
		std::ofstream file(path);
		file << content;
	}

	static std::string readAssetPath(const configuru::Config& projectJson, const char* systemName, int index,
									 const char* propertyName)
	{
		return projectJson[systemName]["components"][index][propertyName].get_or<std::string>("asset_path", "");
	}
};
} // namespace

bool legacy_graph_migration_test_moves_files_and_paths()
{
	UNIT_TEST_BEGIN("moves legacy graphs and rewrites project paths")

	MigrationTestProject project;

	success&= LegacyGraphMigration::migrateProject(project.projectDir, project.projectFilePath);

	// Files sorted by class into the new folders with the new extensions
	success&= std::filesystem::exists(project.projectDir / "compositors" / "comp.compgraph");
	success&= std::filesystem::exists(project.projectDir / "shapes" / "quad.shapegraph");
	success&= !std::filesystem::exists(project.projectDir / "graphs");

	// The material graph renamed in place, and its .mat follows
	const std::filesystem::path materialFolder= project.projectDir / "shaders" / "compositor" / "m";
	success&= std::filesystem::exists(materialFolder / "m.matgraph");
	success&= !std::filesystem::exists(materialFolder / "m.graph");
	const configuru::Config materialJson= configuru::parse_file((materialFolder / "m.mat").string(), configuru::JSON);
	success&= materialJson.get_or<std::string>("sourceGraphPath", "") == "m.matgraph";

	// Project file paths in the current form, the bundled reference included
	const configuru::Config projectJson= configuru::parse_file(project.projectFilePath.string(), configuru::JSON);
	success&= MigrationTestProject::readAssetPath(projectJson, "CompositorObjectSystem", 0, "compositor_graph_path")
			  == "compositors/comp.compgraph";
	success&= MigrationTestProject::readAssetPath(projectJson, "CompositorObjectSystem", 1, "compositor_graph_path")
			  == "compositors/color_key_graph.compgraph";
	success&= MigrationTestProject::readAssetPath(projectJson, "QuadShapeSystem", 0, "shape_graph_path")
			  == "shapes/quad.shapegraph";

	// A second run finds nothing to do
	success&= !LegacyGraphMigration::migrateProject(project.projectDir, project.projectFilePath);

	UNIT_TEST_COMPLETE()
}

bool legacy_graph_migration_test_path_mapping()
{
	UNIT_TEST_BEGIN("stored path mapping")

	success&= LegacyGraphMigration::migrateCompositorGraphPath("graphs/a.graph") == "compositors/a.compgraph";
	success&= LegacyGraphMigration::migrateShapeGraphPath("graphs/shapes/b.graph") == "shapes/b.shapegraph";
	// An absolute legacy path maps by stem too
	success&=
		LegacyGraphMigration::migrateCompositorGraphPath("D:/somewhere/graphs/c.graph") == "compositors/c.compgraph";
	// Current paths and empty paths pass through
	success&= LegacyGraphMigration::migrateCompositorGraphPath("compositors/a.compgraph") == "compositors/a.compgraph";
	success&= LegacyGraphMigration::migrateShapeGraphPath("").empty();

	UNIT_TEST_COMPLETE()
}

bool run_legacy_graph_migration_tests()
{
	UNIT_TEST_MODULE_BEGIN("legacy_graph_migration")
	UNIT_TEST_MODULE_CALL_TEST(legacy_graph_migration_test_path_mapping);
	UNIT_TEST_MODULE_CALL_TEST(legacy_graph_migration_test_moves_files_and_paths);
	UNIT_TEST_MODULE_END()
}
