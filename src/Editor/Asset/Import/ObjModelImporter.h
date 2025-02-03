#pragma once

#include "IModelImporter.h"

class ObjModelImporter : public IModelImporter
{
public:
	ObjModelImporter() = default;
	ObjModelImporter(class GlModelResourceManager* ownerManager) : IModelImporter(ownerManager) {}

	virtual GlRenderModelResourcePtr importModelFromFile(
		const std::filesystem::path& modelPath,
		GlMaterialConstPtr overrideMaterial) override;
};