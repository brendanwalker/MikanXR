#pragma once

#include "MaterialAssetReference.h"
#include "MaterialCompiler/MaterialDomain.h"

// A material that shades the compositor's fullscreen layer quad, the project's
// compositor_materials/ folder
class CompositorMaterialAssetReference : public MaterialAssetReference
{
public:
	CompositorMaterialAssetReference()= default;

	inline static const std::string k_assetClassName= "CompositorMaterialAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "CompositorMaterial"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_PAINT_BRUSH; }
};

class CompositorMaterialAssetReferenceFactory
	: public TypedAssetReferenceFactory<CompositorMaterialAssetReference, AssetReferenceConfig>
{
public:
	CompositorMaterialAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "CompositorMaterial"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadCompositorMaterialDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1]= {MaterialDomainUtils::k_compositorMaterialFilterPattern};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return locText("assets.compositorMaterialFilterDescription"); }

	virtual bool editorCanCreate() const { return true; }
};
