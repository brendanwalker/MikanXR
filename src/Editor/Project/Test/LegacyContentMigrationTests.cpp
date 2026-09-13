#include "LegacyContentMigrationTests.h"
#include "unit_test.h"

#include "LegacyContentMigration.h"
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

		root= std::filesystem::temp_directory_path() / "mikan_legacy_content_migration_tests";
		std::filesystem::remove_all(root);
		projectDir= root / "project";
		projectFilePath= projectDir / "project.mikanproj";

		std::filesystem::create_directories(projectDir / "graphs" / "shapes");
		std::filesystem::create_directories(projectDir / "shaders" / "compositor" / "m");
		std::filesystem::create_directories(projectDir / "shaders" / "shape" / "s");

		// A compositor graph referencing both materials, and a shape graph
		writeFile(projectDir / "graphs" / "comp.graph", R"({
	"class_name": "CompositorNodeGraph",
	"assetReferences": [
		{ "class_name": "MaterialAssetReference", "asset_path": "shaders/compositor/m/m.mat" },
		{ "class_name": "MaterialAssetReference", "asset_path": "shaders/compositor/rgbFrame/rgbFrame.mat" },
		{ "class_name": "MaterialAssetReference", "asset_path": "shaders/shape/s/s.mat" },
		{ "class_name": "TextureAssetReference", "asset_path": "textures/grid.png" }
	],
	"nodes": []
})");
		writeFile(projectDir / "graphs" / "shapes" / "quad.graph",
				  R"({ "class_name": "ShapeNodeGraph", "assetReferences": [], "nodes": [] })");

		// A compositor material with a graph and a hand-authored shape material without one
		writeFile(projectDir / "shaders" / "compositor" / "m" / "m.graph",
				  R"({ "class_name": "MaterialNodeGraph", "nodes": [] })");
		writeFile(
			projectDir / "shaders" / "compositor" / "m" / "m.mat",
			R"({ "materialName": "m", "vertexAttributes": [], "uniformSemanticMap": {}, "domain": "compositor", "sourceGraphPath": "m.graph" })");
		writeFile(projectDir / "shaders" / "compositor" / "m" / "m.vert", "");
		writeFile(projectDir / "shaders" / "shape" / "s" / "s.mat",
				  R"({ "materialName": "s", "vertexAttributes": [], "uniformSemanticMap": {}, "domain": "shape" })");

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

bool legacy_content_migration_test_moves_files_and_paths()
{
	UNIT_TEST_BEGIN("moves legacy graphs and materials and rewrites references")

	MigrationTestProject project;

	success&= LegacyContentMigration::migrateProject(project.projectDir, project.projectFilePath);

	// Graphs sorted by class into the new folders with the new extensions
	success&= std::filesystem::exists(project.projectDir / "compositors" / "comp.compgraph");
	success&= std::filesystem::exists(project.projectDir / "shapes" / "quad.shapegraph");
	success&= !std::filesystem::exists(project.projectDir / "graphs");

	// Material folders under their domain folder, the material file and graph renamed
	const std::filesystem::path compositorMaterial= project.projectDir / "compositor_materials" / "m";
	success&= std::filesystem::exists(compositorMaterial / "m.compmat");
	success&= std::filesystem::exists(compositorMaterial / "m.matgraph");
	success&= std::filesystem::exists(compositorMaterial / "m.vert");
	const configuru::Config materialJson=
		configuru::parse_file((compositorMaterial / "m.compmat").string(), configuru::JSON);
	success&= materialJson.get_or<std::string>("sourceGraphPath", "") == "m.matgraph";
	success&= std::filesystem::exists(project.projectDir / "shape_materials" / "s" / "s.shapemat");
	success&= !std::filesystem::exists(project.projectDir / "shaders");

	// The graph's material references carry the domain class and path; other references are untouched
	const configuru::Config graphJson=
		configuru::parse_file((project.projectDir / "compositors" / "comp.compgraph").string(), configuru::JSON);
	const auto& assetRefs= graphJson["assetReferences"].as_array();
	success&= assetRefs.size() == 4;
	success&= assetRefs[0].get_or<std::string>("class_name", "") == "CompositorMaterialAssetReference"
			  && assetRefs[0].get_or<std::string>("asset_path", "") == "compositor_materials/m/m.compmat";
	success&= assetRefs[1].get_or<std::string>("asset_path", "") == "compositor_materials/rgbFrame/rgbFrame.compmat";
	success&= assetRefs[2].get_or<std::string>("class_name", "") == "ShapeMaterialAssetReference"
			  && assetRefs[2].get_or<std::string>("asset_path", "") == "shape_materials/s/s.shapemat";
	success&= assetRefs[3].get_or<std::string>("class_name", "") == "TextureAssetReference"
			  && assetRefs[3].get_or<std::string>("asset_path", "") == "textures/grid.png";

	// Project file paths in the current form, the bundled reference included
	const configuru::Config projectJson= configuru::parse_file(project.projectFilePath.string(), configuru::JSON);
	success&= MigrationTestProject::readAssetPath(projectJson, "CompositorObjectSystem", 0, "compositor_graph_path")
			  == "compositors/comp.compgraph";
	success&= MigrationTestProject::readAssetPath(projectJson, "CompositorObjectSystem", 1, "compositor_graph_path")
			  == "compositors/color_key_graph.compgraph";
	success&= MigrationTestProject::readAssetPath(projectJson, "QuadShapeSystem", 0, "shape_graph_path")
			  == "shapes/quad.shapegraph";

	// A second run finds nothing to do
	success&= !LegacyContentMigration::migrateProject(project.projectDir, project.projectFilePath);

	UNIT_TEST_COMPLETE()
}

bool legacy_content_migration_test_path_mapping()
{
	UNIT_TEST_BEGIN("stored path mapping")

	success&= LegacyContentMigration::migrateCompositorGraphPath("graphs/a.graph") == "compositors/a.compgraph";
	success&= LegacyContentMigration::migrateShapeGraphPath("graphs/shapes/b.graph") == "shapes/b.shapegraph";
	// An absolute legacy path maps by stem too
	success&=
		LegacyContentMigration::migrateCompositorGraphPath("D:/somewhere/graphs/c.graph") == "compositors/c.compgraph";
	// Current paths and empty paths pass through
	success&=
		LegacyContentMigration::migrateCompositorGraphPath("compositors/a.compgraph") == "compositors/a.compgraph";
	success&= LegacyContentMigration::migrateShapeGraphPath("").empty();

	// Materials map by the domain folder in their path
	LegacyContentMigration::MaterialPathMigration migration=
		LegacyContentMigration::migrateMaterialPath("shaders/compositor/x/x.mat");
	success&= migration.storedPath == "compositor_materials/x/x.compmat"
			  && migration.assetClassName == "CompositorMaterialAssetReference";
	migration= LegacyContentMigration::migrateMaterialPath("D:/dev/resources/shaders/shape/y/y.mat");
	success&= migration.storedPath == "shape_materials/y/y.shapemat"
			  && migration.assetClassName == "ShapeMaterialAssetReference";
	// No domain folder, or already current: unchanged and unclassified
	migration= LegacyContentMigration::migrateMaterialPath("somewhere/z.mat");
	success&= migration.storedPath == "somewhere/z.mat" && migration.assetClassName.empty();
	migration= LegacyContentMigration::migrateMaterialPath("compositor_materials/x/x.compmat");
	success&= migration.storedPath == "compositor_materials/x/x.compmat" && migration.assetClassName.empty();

	UNIT_TEST_COMPLETE()
}

bool run_legacy_content_migration_tests()
{
	UNIT_TEST_MODULE_BEGIN("legacy_content_migration")
	UNIT_TEST_MODULE_CALL_TEST(legacy_content_migration_test_path_mapping);
	UNIT_TEST_MODULE_CALL_TEST(legacy_content_migration_test_moves_files_and_paths);
	UNIT_TEST_MODULE_END()
}
