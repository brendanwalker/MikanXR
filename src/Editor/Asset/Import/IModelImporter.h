#pragma once

#include <memory>
#include <filesystem>

class MikanRenderModelResource;
using MikanRenderModelResourcePtr = std::shared_ptr<MikanRenderModelResource>;

class MkMaterial;
using MkMaterialConstPtr = std::shared_ptr<const MkMaterial>;

class IModelImporter
{
public:
	IModelImporter(class MikanModelResourceManager* ownerManager) : m_ownerManager(ownerManager) {}
	virtual ~IModelImporter() {}

	virtual MikanRenderModelResourcePtr importModelFromFile(
		const std::filesystem::path& modelPath,
		MkMaterialConstPtr overrideMaterial) = 0;

protected:
	class MikanModelResourceManager* m_ownerManager;
};

using IModelImporterPtr = std::shared_ptr<IModelImporter>;