#pragma once

#include <memory>
#include <filesystem>

class MikanRenderModelResource;
using MikanRenderModelResourcePtr = std::shared_ptr<MikanRenderModelResource>;

class MkMaterial;
using MkMaterialConstPtr = std::shared_ptr<const MkMaterial>;

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