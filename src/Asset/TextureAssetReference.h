#pragma once

#include "AssetReference.h"

class TextureAssetReference : public AssetReference
{
public:
	TextureAssetReference() = default;

	virtual std::string getAssetTypeName() const override { return "Texture"; }

	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class TextureAssetReferenceFactory : public AssetReferenceFactory
{
public:
	TextureAssetReferenceFactory() = default;

	virtual std::string getAssetTypeName() const { return "Texture"; }
	virtual char const* getFileDialogTitle() const { return "Load Texture"; }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[5] = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tga"};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 5; }
	virtual char const* getFilterDescription() const { return "Image Files (*.jpg;*.jpeg;*.png;*.bmp;*.tga)"; }

	virtual std::shared_ptr<AssetReference> createAssetReference(
		const class NodeEditorState* editorState,
		const std::filesystem::path& inAssetPath) const
	{
		auto assetRef = std::make_shared<TextureAssetReference>();
		assetRef->setAssetPath(inAssetPath);

		return assetRef;
	}
};