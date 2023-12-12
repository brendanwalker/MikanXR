#pragma once

#include "AssetReference.h"

class MaterialAssetReference : public AssetReference
{
public:
	MaterialAssetReference() = default;

	virtual std::string getAssetTypeName() const override { return "Material"; }

protected:
	virtual void rebuildPreview() override;
};

class MaterialAssetReferenceFactory : public AssetReferenceFactory
{
public:
	MaterialAssetReferenceFactory() = default;

	virtual std::string getAssetTypeName() const { return "Material"; }
	virtual char const* getFileDialogTitle() const { return "Load Material"; }
	virtual char const* const* getFilterPatterns() const { 
		static const char* filterItems[1] = {"*.mat"};
		return filterItems; 
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return "Material Files (*.mat)"; }

	virtual std::shared_ptr<AssetReference> createAssetReference(
		const class NodeEditorState* editorState,
		const std::filesystem::path& inAssetPath) const
	{
		auto assetRef= std::make_shared<MaterialAssetReference>();
		assetRef->setAssetPath(inAssetPath);

		return assetRef;
	}
};