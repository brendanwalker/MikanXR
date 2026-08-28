#pragma once

#include "AssetReference.h"
#include "LocText.h"

class MaterialAssetReference : public AssetReference
{
public:
	MaterialAssetReference()= default;

	inline static const std::string k_assetClassName= "MaterialAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "Material"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_PAINT_BRUSH; }

	virtual void editorHandleGraphVariablesDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class MaterialAssetReferenceFactory : public TypedAssetReferenceFactory<MaterialAssetReference, AssetReferenceConfig>
{
public:
	MaterialAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "Material"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadMaterialDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1]= {"*.mat"};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return locText("assets.materialFilterDescription"); }

	virtual bool editorCanCreate() const { return true; }

	static std::string getDefaultMaterialPath();
};