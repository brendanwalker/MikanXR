#pragma once

#include <memory>
#include <filesystem>

class MikanRenderModelResource;
using MikanRenderModelResourcePtr = std::shared_ptr<MikanRenderModelResource>;

class GlMaterial;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class IModelExporter
{
public:
	IModelExporter(class MikanModelResourceManager* ownerManager) : m_ownerManager(ownerManager) {}
	virtual ~IModelExporter() {}

	virtual bool exportModelToFile(
		MikanRenderModelResourcePtr modelResource,
		const std::filesystem::path& modelPath) = 0;

protected:
	class MikanModelResourceManager* m_ownerManager;
};

using IModelExporterPtr = std::shared_ptr<IModelExporter>;