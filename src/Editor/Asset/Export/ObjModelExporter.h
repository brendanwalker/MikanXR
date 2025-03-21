#pragma once

#include "IModelExporter.h"

class ObjModelExporter : public IModelExporter
{
public:
	ObjModelExporter() = default;
	ObjModelExporter(class MikanModelResourceManager* ownerManager) : IModelExporter(ownerManager) {}

	virtual bool exportModelToFile(
		MikanRenderModelResourcePtr modelResource,
		const std::filesystem::path& modelPath) override;
};