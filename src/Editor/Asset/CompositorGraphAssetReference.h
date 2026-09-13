#pragma once

#include "AssetReference.h"
#include "Graphs/NodeGraphFileTypes.h"
#include "LocText.h"

// A compositor node graph file, the project's compositors/ folder
class CompositorGraphAssetReference : public AssetReference
{
public:
	CompositorGraphAssetReference()= default;

	inline static const std::string k_assetClassName= "CompositorGraphAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "CompositorGraph"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_SITEMAP; }

	// Opens the graph in the compositor editor, bound to the compositor that
	// drives it when one does so the graph evaluates live
	virtual bool editorCanOpen() const override { return !isEmpty(); }
	virtual void editorOpen() override;
};

class CompositorGraphAssetReferenceFactory
	: public TypedAssetReferenceFactory<CompositorGraphAssetReference, AssetReferenceConfig>
{
public:
	CompositorGraphAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "CompositorGraph"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadCompositorGraphDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1]= {NodeGraphFileTypes::k_compositorGraphFilterPattern};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return locText("assets.compositorGraphFilterDescription"); }
};
