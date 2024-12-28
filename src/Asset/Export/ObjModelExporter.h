#pragma once

#include "IModelExporter.h"

class ObjModelExporter : public IModelExporter
{
public:
	ObjModelExporter() = default;
	ObjModelExporter(class GlModelResourceManager* ownerManager) : IModelExporter(ownerManager) {}

	virtual bool exportModelToFile(
		GlRenderModelResourcePtr modelResource,
		const std::filesystem::path& modelPath) override;
};