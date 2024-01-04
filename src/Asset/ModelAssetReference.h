#pragma once

#include "AssetReference.h"

class ModelAssetReference : public AssetReference
{
public:
	ModelAssetReference() = default;

	inline static const std::string k_assetClassName = "ModelAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "Model"; }
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class ModelAssetReferenceFactory : 
	public TypedAssetReferenceFactory<ModelAssetReference, AssetReferenceConfig>
{
public:
	ModelAssetReferenceFactory() = default;

	virtual std::string getAssetTypeName() const { return "Model"; }
	virtual char const* getFileDialogTitle() const { return "Load Model"; }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1] = {"*.obj"};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return "Model Files (*.obj)"; }
};