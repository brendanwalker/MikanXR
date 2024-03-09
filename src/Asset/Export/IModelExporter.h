#pragma once

#include <memory>
#include <filesystem>

class GlRenderModelResource;
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;

class GlMaterial;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class IModelExporter
{
public:
	IModelExporter(class GlModelResourceManager* ownerManager) : m_ownerManager(ownerManager) {}
	virtual ~IModelExporter() {}

	virtual bool exportModelToFile(
		GlRenderModelResourcePtr modelResource,
		const std::filesystem::path& modelPath) = 0;

protected:
	class GlModelResourceManager* m_ownerManager;
};

using IModelExporterPtr = std::shared_ptr<IModelExporter>;