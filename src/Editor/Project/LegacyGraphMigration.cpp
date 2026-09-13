#include "LegacyGraphMigration.h"
#include "Logger.h"
#include "MikanShaderConfig.h"
#include "PathUtils.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeGraphFileTypes.h"
#include "Graphs/ShapeNodeGraph.h"

#include <configuru.hpp>

#include <vector>

namespace LegacyGraphMigration
{
static std::string migrateGraphPath(const std::string& storedPath, const char* folder, const char* extension)
{
	if (storedPath.empty())
	{
		return storedPath;
	}

	const std::filesystem::path path(storedPath);
	if (path.extension() != NodeGraphFileTypes::k_legacyGraphExtension)
	{
		return storedPath;
	}

	return (std::filesystem::path(folder) / (path.stem().string() + extension)).generic_string();
}

std::string migrateCompositorGraphPath(const std::string& storedPath)
{
	return migrateGraphPath(storedPath, "compositors", NodeGraphFileTypes::k_compositorGraphExtension);
}

std::string migrateShapeGraphPath(const std::string& storedPath)
{
	return migrateGraphPath(storedPath, "shapes", NodeGraphFileTypes::k_shapeGraphExtension);
}

// Moves one legacy graph file into the folder its class belongs to, skipping a
// file whose destination is taken so nothing is overwritten
static bool moveLegacyGraph(const std::filesystem::path& projectDir, const std::filesystem::path& graphPath)
{
	const std::string className= NodeGraphFactory::peekGraphClassName(graphPath);

	std::filesystem::path destPath;
	if (className == CompositorNodeGraph::k_graphClassName)
	{
		destPath=
			projectDir / "compositors" / (graphPath.stem().string() + NodeGraphFileTypes::k_compositorGraphExtension);
	}
	else if (className == ShapeNodeGraph::k_graphClassName)
	{
		destPath= projectDir / "shapes" / (graphPath.stem().string() + NodeGraphFileTypes::k_shapeGraphExtension);
	}
	else
	{
		MIKAN_LOG_WARNING("LegacyGraphMigration")
			<< "Leaving graph of class '" << className << "' in place, no folder for it: " << graphPath.string();
		return false;
	}

	std::error_code ec;
	if (std::filesystem::exists(destPath, ec))
	{
		MIKAN_LOG_WARNING("LegacyGraphMigration")
			<< "Leaving " << graphPath.string() << " in place, " << destPath.string() << " already exists";
		return false;
	}

	std::filesystem::create_directories(destPath.parent_path(), ec);
	std::filesystem::rename(graphPath, destPath, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyGraphMigration") << "Failed to move " << graphPath.string() << ": " << ec.message();
		return false;
	}

	MIKAN_LOG_INFO("LegacyGraphMigration") << "Moved " << graphPath.string() << " to " << destPath.string();
	return true;
}

// Renames a material's legacy graph beside its .mat and points the .mat at the new name
static bool renameLegacyMaterialGraph(const std::filesystem::path& graphPath)
{
	const std::filesystem::path destPath=
		graphPath.parent_path() / (graphPath.stem().string() + NodeGraphFileTypes::k_materialGraphExtension);

	std::error_code ec;
	if (std::filesystem::exists(destPath, ec))
	{
		MIKAN_LOG_WARNING("LegacyGraphMigration")
			<< "Leaving " << graphPath.string() << " in place, " << destPath.string() << " already exists";
		return false;
	}

	std::filesystem::rename(graphPath, destPath, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyGraphMigration") << "Failed to rename " << graphPath.string() << ": " << ec.message();
		return false;
	}

	// The .mat names its graph by filename, so a .mat beside it that named the
	// old file is rewritten to the new one
	const std::filesystem::path materialPath= graphPath.parent_path() / (graphPath.stem().string() + ".mat");
	if (std::filesystem::exists(materialPath, ec))
	{
		try
		{
			configuru::Config materialJson= configuru::parse_file(materialPath.string(), configuru::JSON);
			const std::string sourceGraphPath= materialJson.get_or<std::string>("sourceGraphPath", "");
			if (sourceGraphPath == graphPath.filename().string())
			{
				materialJson["sourceGraphPath"]= destPath.filename().string();
				configuru::dump_file(materialPath.string(), materialJson, configuru::JSON);
			}
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("LegacyGraphMigration") << "Failed to update " << materialPath.string() << ": " << e.what();
		}
	}

	MIKAN_LOG_INFO("LegacyGraphMigration") << "Renamed " << graphPath.string() << " to " << destPath.string();
	return true;
}

// Rewrites every graph path property in the project file, wherever it sits in the
// definition tree, from its legacy form to the current one
static bool migrateProjectFilePaths(configuru::Config& node)
{
	bool bChanged= false;

	if (node.is_object())
	{
		for (auto& entry : node.as_object())
		{
			const std::string& key= entry.key();
			configuru::Config& value= entry.value();

			if ((key == "compositor_graph_path" || key == "shape_graph_path") && value.is_object()
				&& value.has_key("asset_path"))
			{
				const std::string assetPath= value.get_or<std::string>("asset_path", "");
				const std::string migrated= key == "compositor_graph_path" ? migrateCompositorGraphPath(assetPath)
																		   : migrateShapeGraphPath(assetPath);
				if (migrated != assetPath)
				{
					value["asset_path"]= migrated;
					bChanged= true;
				}
			}
			else
			{
				bChanged|= migrateProjectFilePaths(value);
			}
		}
	}
	else if (node.is_array())
	{
		for (configuru::Config& element : node.as_array())
		{
			bChanged|= migrateProjectFilePaths(element);
		}
	}

	return bChanged;
}

bool migrateProject(const std::filesystem::path& projectDir, const std::filesystem::path& projectFilePath)
{
	bool bChanged= false;
	std::error_code ec;

	// The shared graphs/ folder: each file goes to the folder of its class
	const std::filesystem::path legacyGraphsDir= projectDir / "graphs";
	if (std::filesystem::is_directory(legacyGraphsDir, ec))
	{
		std::vector<std::filesystem::path> legacyGraphs;
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(legacyGraphsDir, ec))
		{
			if (dirEntry.is_regular_file(ec)
				&& dirEntry.path().extension() == NodeGraphFileTypes::k_legacyGraphExtension)
			{
				legacyGraphs.push_back(dirEntry.path());
			}
		}

		for (const std::filesystem::path& graphPath : legacyGraphs)
		{
			bChanged|= moveLegacyGraph(projectDir, graphPath);
		}

		// Emptied folders go, the ones still holding something stay
		std::vector<std::filesystem::path> subfolders;
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(legacyGraphsDir, ec))
		{
			if (dirEntry.is_directory(ec))
				subfolders.push_back(dirEntry.path());
		}
		for (auto it= subfolders.rbegin(); it != subfolders.rend(); ++it)
		{
			if (std::filesystem::is_empty(*it, ec))
				std::filesystem::remove(*it, ec);
		}
		if (std::filesystem::is_empty(legacyGraphsDir, ec))
		{
			std::filesystem::remove(legacyGraphsDir, ec);
		}
	}

	// Material graphs beside their .mat under shaders/
	const std::filesystem::path shadersDir= projectDir / "shaders";
	if (std::filesystem::is_directory(shadersDir, ec))
	{
		std::vector<std::filesystem::path> legacyMaterialGraphs;
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(shadersDir, ec))
		{
			if (dirEntry.is_regular_file(ec)
				&& dirEntry.path().extension() == NodeGraphFileTypes::k_legacyGraphExtension)
			{
				legacyMaterialGraphs.push_back(dirEntry.path());
			}
		}

		for (const std::filesystem::path& graphPath : legacyMaterialGraphs)
		{
			bChanged|= renameLegacyMaterialGraph(graphPath);
		}
	}

	// The stored paths in the project file
	if (std::filesystem::is_regular_file(projectFilePath, ec))
	{
		try
		{
			configuru::Config projectJson= configuru::parse_file(projectFilePath.string(), configuru::JSON);
			if (migrateProjectFilePaths(projectJson))
			{
				configuru::dump_file(projectFilePath.string(), projectJson, configuru::JSON);
				MIKAN_LOG_INFO("LegacyGraphMigration") << "Rewrote graph paths in " << projectFilePath.string();
				bChanged= true;
			}
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("LegacyGraphMigration")
				<< "Failed to migrate graph paths in " << projectFilePath.string() << ": " << e.what();
		}
	}

	return bChanged;
}
} // namespace LegacyGraphMigration
