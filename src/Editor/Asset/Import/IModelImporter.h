#pragma once

#include <memory>
#include <filesystem>

class MikanRenderModelResource;
using MikanRenderModelResourcePtr = std::shared_ptr<MikanRenderModelResource>;

class GlMaterial;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class IModelImporter
{
public:
	IModelImporter(class MikanModelResourceManager* ownerManager) : m_ownerManager(ownerManager) {}
	virtual ~IModelImporter() {}

	virtual MikanRenderModelResourcePtr importModelFromFile(
		const std::filesystem::path& modelPath,
		GlMaterialConstPtr overrideMaterial) = 0;

protected:
	class MikanModelResourceManager* m_ownerManager;
};

using IModelImporterPtr = std::shared_ptr<IModelImporter>;