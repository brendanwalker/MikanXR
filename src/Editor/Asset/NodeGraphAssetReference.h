#pragma once

#include "AssetReference.h"

class NodeGraphAssetReference : public AssetReference
{
public:
	NodeGraphAssetReference() = default;

	inline static const std::string k_assetClassName = "NodeGraphAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "NodeGraph"; }

	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	virtual void rebuildPreview() override;
};

class NodeGraphAssetReferenceFactory : 
	public TypedAssetReferenceFactory<NodeGraphAssetReference, AssetReferenceConfig>
{
public:
	NodeGraphAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "NodeGraph"; }
	virtual char const* getFileDialogTitle() const { return "Load NodeGraph"; }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[5] = {"*.graph"};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 5; }
	virtual char const* getFilterDescription() const { return "Node Graph Files (*.graph)"; }
};