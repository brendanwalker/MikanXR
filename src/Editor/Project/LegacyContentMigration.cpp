#include "LegacyContentMigration.h"
#include "CompositorMaterialAssetReference.h"
#include "Logger.h"
#include "MikanShaderConfig.h"
#include "PathUtils.h"
#include "ShapeMaterialAssetReference.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeGraphFileTypes.h"
#include "Graphs/ShapeNodeGraph.h"
#include "MaterialCompiler/MaterialDomain.h"

#include <configuru.hpp>

#include <vector>

namespace LegacyContentMigration
{
// The asset reference class every material carried before the split
static const char* k_legacyMaterialAssetClassName= "MaterialAssetReference";

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

// The domain a legacy material path names through its shaders/<domain>/ folder
static eMaterialDomain domainFromLegacyMaterialPath(const std::filesystem::path& path)
{
	const std::filesystem::path normal= path.lexically_normal();
	for (auto it= normal.begin(); it != normal.end(); ++it)
	{
		if (it->string() != "shaders")
			continue;

		auto domainIt= std::next(it);
		if (domainIt != normal.end())
		{
			return MaterialDomainUtils::domainFromString(domainIt->string());
		}
	}

	return eMaterialDomain::INVALID;
}

MaterialPathMigration migrateMaterialPath(const std::string& storedPath)
{
	MaterialPathMigration migration;
	migration.storedPath= storedPath;

	const std::filesystem::path path(storedPath);
	if (storedPath.empty() || path.extension() != MaterialDomainUtils::k_legacyMaterialExtension)
	{
		return migration;
	}

	const eMaterialDomain domain= domainFromLegacyMaterialPath(path);
	if (domain == eMaterialDomain::INVALID)
	{
		return migration;
	}

	const std::string stem= path.stem().string();
	migration.storedPath= (std::filesystem::path(MaterialDomainUtils::materialFolderName(domain)) / stem
						   / (stem + MaterialDomainUtils::materialExtension(domain)))
							  .generic_string();
	migration.assetClassName= domain == eMaterialDomain::compositor ? CompositorMaterialAssetReference::k_assetClassName
																	: ShapeMaterialAssetReference::k_assetClassName;

	return migration;
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
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving graph of class '" << className << "' in place, no folder for it: " << graphPath.string();
		return false;
	}

	std::error_code ec;
	if (std::filesystem::exists(destPath, ec))
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving " << graphPath.string() << " in place, " << destPath.string() << " already exists";
		return false;
	}

	std::filesystem::create_directories(destPath.parent_path(), ec);
	std::filesystem::rename(graphPath, destPath, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyContentMigration") << "Failed to move " << graphPath.string() << ": " << ec.message();
		return false;
	}

	MIKAN_LOG_INFO("LegacyContentMigration") << "Moved " << graphPath.string() << " to " << destPath.string();
	return true;
}

// Rewrites a JSON string field when the new value differs
static bool rewriteStringField(configuru::Config& object, const char* key, const std::string& newValue)
{
	if (object.get_or<std::string>(key, "") == newValue)
	{
		return false;
	}

	object[key]= newValue;
	return true;
}

// Renames a legacy material graph beside its material file and points the
// material file at the new name
static bool renameLegacyMaterialGraph(const std::filesystem::path& graphPath)
{
	const std::filesystem::path destPath=
		graphPath.parent_path() / (graphPath.stem().string() + NodeGraphFileTypes::k_materialGraphExtension);

	std::error_code ec;
	if (std::filesystem::exists(destPath, ec))
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving " << graphPath.string() << " in place, " << destPath.string() << " already exists";
		return false;
	}

	std::filesystem::rename(graphPath, destPath, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyContentMigration") << "Failed to rename " << graphPath.string() << ": " << ec.message();
		return false;
	}

	// The material file names its graph by filename. It may still carry the
	// legacy extension at this point, since materials move after graphs.
	for (const std::filesystem::path& materialPath :
		 {graphPath.parent_path() / (graphPath.stem().string() + MaterialDomainUtils::k_legacyMaterialExtension),
		  graphPath.parent_path()
			  / (graphPath.stem().string() + MaterialDomainUtils::materialExtension(eMaterialDomain::compositor)),
		  graphPath.parent_path()
			  / (graphPath.stem().string() + MaterialDomainUtils::materialExtension(eMaterialDomain::shape))})
	{
		if (!std::filesystem::exists(materialPath, ec))
			continue;

		try
		{
			configuru::Config materialJson= configuru::parse_file(materialPath.string(), configuru::JSON);
			if (materialJson.get_or<std::string>("sourceGraphPath", "") == graphPath.filename().string())
			{
				materialJson["sourceGraphPath"]= destPath.filename().string();
				configuru::dump_file(materialPath.string(), materialJson, configuru::JSON);
			}
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("LegacyContentMigration")
				<< "Failed to update " << materialPath.string() << ": " << e.what();
		}
	}

	MIKAN_LOG_INFO("LegacyContentMigration") << "Renamed " << graphPath.string() << " to " << destPath.string();
	return true;
}

// Moves one legacy material folder under the folder of its domain, renaming the
// material file to the domain's extension
static bool moveLegacyMaterial(const std::filesystem::path& projectDir, const std::filesystem::path& materialPath)
{
	MikanShaderConfig materialConfig;
	if (!materialConfig.load(materialPath))
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving unreadable material in place: " << materialPath.string();
		return false;
	}

	eMaterialDomain domain= MaterialDomainUtils::resolveMaterialDomain(materialConfig);
	if (domain == eMaterialDomain::INVALID)
	{
		domain= domainFromLegacyMaterialPath(materialPath);
	}
	if (domain == eMaterialDomain::INVALID)
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving material of unknown domain in place: " << materialPath.string();
		return false;
	}

	const std::string stem= materialPath.stem().string();
	const std::filesystem::path sourceFolder= materialPath.parent_path();
	const std::filesystem::path destFolder= projectDir / MaterialDomainUtils::materialFolderName(domain) / stem;

	std::error_code ec;
	if (std::filesystem::exists(destFolder, ec))
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Leaving " << materialPath.string() << " in place, " << destFolder.string() << " already exists";
		return false;
	}

	std::filesystem::create_directories(destFolder.parent_path(), ec);
	std::filesystem::rename(sourceFolder, destFolder, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyContentMigration") << "Failed to move " << sourceFolder.string() << ": " << ec.message();
		return false;
	}

	const std::filesystem::path movedMaterialPath= destFolder / materialPath.filename();
	const std::filesystem::path renamedMaterialPath=
		destFolder / (stem + MaterialDomainUtils::materialExtension(domain));
	std::filesystem::rename(movedMaterialPath, renamedMaterialPath, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LegacyContentMigration")
			<< "Failed to rename " << movedMaterialPath.string() << ": " << ec.message();
		return false;
	}

	MIKAN_LOG_INFO("LegacyContentMigration")
		<< "Moved " << materialPath.string() << " to " << renamedMaterialPath.string();
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

// Rewrites the material references a compositor or shape graph file holds
static bool migrateGraphFileMaterialReferences(const std::filesystem::path& graphPath)
{
	configuru::Config graphJson;
	try
	{
		graphJson= configuru::parse_file(graphPath.string(), configuru::JSON);
	}
	catch (const std::exception& e)
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "Skipping unreadable graph " << graphPath.string() << ": " << e.what();
		return false;
	}

	if (!graphJson.has_key("assetReferences") || !graphJson["assetReferences"].is_array())
	{
		return false;
	}

	bool bChanged= false;
	for (configuru::Config& assetRef : graphJson["assetReferences"].as_array())
	{
		if (assetRef.get_or<std::string>("class_name", "") != k_legacyMaterialAssetClassName)
			continue;

		const std::string assetPath= assetRef.get_or<std::string>("asset_path", "");
		const MaterialPathMigration migration= migrateMaterialPath(assetPath);
		if (migration.assetClassName.empty())
		{
			MIKAN_LOG_WARNING("LegacyContentMigration")
				<< "Leaving material reference with no domain folder in " << graphPath.string() << ": " << assetPath;
			continue;
		}

		bChanged|= rewriteStringField(assetRef, "asset_path", migration.storedPath);
		bChanged|= rewriteStringField(assetRef, "class_name", migration.assetClassName);
	}

	if (bChanged)
	{
		try
		{
			configuru::dump_file(graphPath.string(), graphJson, configuru::JSON);
			MIKAN_LOG_INFO("LegacyContentMigration") << "Rewrote material references in " << graphPath.string();
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("LegacyContentMigration") << "Failed to write " << graphPath.string() << ": " << e.what();
			return false;
		}
	}

	return bChanged;
}

static std::vector<std::filesystem::path> collectFilesWithExtension(const std::filesystem::path& root,
																	const char* extension)
{
	std::vector<std::filesystem::path> files;
	std::error_code ec;
	if (!std::filesystem::is_directory(root, ec))
	{
		return files;
	}

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(root, ec))
	{
		if (dirEntry.is_regular_file(ec) && dirEntry.path().extension() == extension)
		{
			files.push_back(dirEntry.path());
		}
	}

	return files;
}

// Removes the folders under root that are empty, deepest first, then root itself
static void removeEmptyFolders(const std::filesystem::path& root)
{
	std::error_code ec;
	if (!std::filesystem::is_directory(root, ec))
	{
		return;
	}

	std::vector<std::filesystem::path> subfolders;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(root, ec))
	{
		if (dirEntry.is_directory(ec))
			subfolders.push_back(dirEntry.path());
	}
	for (auto it= subfolders.rbegin(); it != subfolders.rend(); ++it)
	{
		if (std::filesystem::is_empty(*it, ec))
			std::filesystem::remove(*it, ec);
	}
	if (std::filesystem::is_empty(root, ec))
	{
		std::filesystem::remove(root, ec);
	}
}

bool migrateProject(const std::filesystem::path& projectDir, const std::filesystem::path& projectFilePath)
{
	bool bChanged= false;
	std::error_code ec;

	// The shared graphs/ folder: each file goes to the folder of its class
	const std::filesystem::path legacyGraphsDir= projectDir / "graphs";
	for (const std::filesystem::path& graphPath :
		 collectFilesWithExtension(legacyGraphsDir, NodeGraphFileTypes::k_legacyGraphExtension))
	{
		bChanged|= moveLegacyGraph(projectDir, graphPath);
	}
	removeEmptyFolders(legacyGraphsDir);

	// The shared shaders/ tree: material graphs get their extension, then each
	// material folder moves under the folder of its domain
	const std::filesystem::path legacyShadersDir= projectDir / "shaders";
	for (const std::filesystem::path& graphPath :
		 collectFilesWithExtension(legacyShadersDir, NodeGraphFileTypes::k_legacyGraphExtension))
	{
		bChanged|= renameLegacyMaterialGraph(graphPath);
	}
	for (const std::filesystem::path& materialPath :
		 collectFilesWithExtension(legacyShadersDir, MaterialDomainUtils::k_legacyMaterialExtension))
	{
		bChanged|= moveLegacyMaterial(projectDir, materialPath);
	}
	removeEmptyFolders(legacyShadersDir);
	if (std::filesystem::is_directory(legacyShadersDir, ec))
	{
		MIKAN_LOG_WARNING("LegacyContentMigration")
			<< "shaders/ still holds files no material owns, left in place: " << legacyShadersDir.string();
	}

	// The material references inside the compositor and shape graphs
	for (const char* extension :
		 {NodeGraphFileTypes::k_compositorGraphExtension, NodeGraphFileTypes::k_shapeGraphExtension})
	{
		for (const std::filesystem::path& graphPath : collectFilesWithExtension(projectDir, extension))
		{
			bChanged|= migrateGraphFileMaterialReferences(graphPath);
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
				MIKAN_LOG_INFO("LegacyContentMigration") << "Rewrote graph paths in " << projectFilePath.string();
				bChanged= true;
			}
		}
		catch (const std::exception& e)
		{
			MIKAN_LOG_ERROR("LegacyContentMigration")
				<< "Failed to migrate graph paths in " << projectFilePath.string() << ": " << e.what();
		}
	}

	return bChanged;
}
} // namespace LegacyContentMigration
