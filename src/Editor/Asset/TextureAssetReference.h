#pragma once

#include "AssetReference.h"

class TextureAssetReference : public AssetReference
{
public:
	TextureAssetReference() = default;

	inline static const std::string k_assetClassName = "TextureAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "Texture"; }

	virtual void editorHandleGraphVariablesDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class TextureAssetReferenceFactory : 
	public TypedAssetReferenceFactory<TextureAssetReference, AssetReferenceConfig>
{
public:
	TextureAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "Texture"; }
	virtual char const* getFileDialogTitle() const { return "Load Texture"; }
	virtual char const* const* getFilterPatterns() const
	{
		return getTextureFilterPatterns();
	}
	virtual int getFilterPatternCount() const { return 5; }
	virtual char const* getFilterDescription() const { return getTextureFilterDescription(); }

	virtual bool editorCanCreate() const { return true; }

	static std::string getDefaultTexturePath();
	static char const* const* getTextureFilterPatterns()
	{
		static const char* filterItems[5] = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tga"};
		return filterItems;
	}
	static int getTextureFilterPatternCount() { return 5; }
	static char const* getTextureFilterDescription() { 
		return "Image Files (*.jpg;*.jpeg;*.png;*.bmp;*.tga)"; 
	}
};