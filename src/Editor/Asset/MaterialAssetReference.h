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

	virtual void setAssetPath(const std::filesystem::path& inPath) override;

	virtual void editorHandleGraphVariablesDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	// True when the .mat records the material graph it was compiled from
	virtual bool editorCanOpen() const override;
	// Opens that graph in the material editor window, creating the window when none is open
	virtual void editorOpen() override;

protected:
	virtual void rebuildPreview() override;

	// The .mat's source graph resolved against the .mat folder, empty when it has
	// none. Read once per asset path, since editorCanOpen is asked every frame.
	const std::filesystem::path& getSourceGraphPath() const;

protected:
	mutable std::filesystem::path m_sourceGraphPath;
	mutable bool m_bSourceGraphPathResolved= false;
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
