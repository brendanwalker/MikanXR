#pragma once

#include <memory>
#include <filesystem>

class GlRenderModelResource;
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;

class GlMaterial;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class IModelImporter
{
public:
	IModelImporter(class GlModelResourceManager* ownerManager) : m_ownerManager(ownerManager) {}
	virtual ~IModelImporter() {}

	virtual GlRenderModelResourcePtr importModelFromFile(
		const std::filesystem::path& modelPath,
		GlMaterialConstPtr overrideMaterial) = 0;

protected:
	class GlModelResourceManager* m_ownerManager;
};

using IModelImporterPtr = std::shared_ptr<IModelImporter>;