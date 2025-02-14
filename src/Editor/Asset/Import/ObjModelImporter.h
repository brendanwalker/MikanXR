#pragma once

#include "IModelImporter.h"

class ObjModelImporter : public IModelImporter
{
public:
	ObjModelImporter() = default;
	ObjModelImporter(class MikanModelResourceManager* ownerManager) : IModelImporter(ownerManager) {}

	virtual MikanRenderModelResourcePtr importModelFromFile(
		const std::filesystem::path& modelPath,
		MkMaterialConstPtr overrideMaterial) override;
};