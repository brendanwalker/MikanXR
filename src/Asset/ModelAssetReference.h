#pragma once

#include "AssetReference.h"

class ModelAssetReference : public AssetReference
{
public:
	ModelAssetReference() = default;

	virtual std::string getAssetTypeName() const override { return "Model"; }
	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class ModelAssetReferenceFactory : public AssetReferenceFactory
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

	virtual std::shared_ptr<AssetReference> createAssetReference(
		const class NodeEditorState* editorState,
		const std::filesystem::path& inAssetPath) const
	{
		auto assetRef = std::make_shared<ModelAssetReference>();
		assetRef->setAssetPath(inAssetPath);

		return assetRef;
	}
};