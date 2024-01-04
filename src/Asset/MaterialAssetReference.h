#pragma once

#include "AssetReference.h"

class MaterialAssetReference : public AssetReference
{
public:
	MaterialAssetReference() = default;

	inline static const std::string k_assetClassName = "MaterialAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "Material"; }
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class MaterialAssetReferenceFactory :
	public TypedAssetReferenceFactory<MaterialAssetReference, AssetReferenceConfig>
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
};