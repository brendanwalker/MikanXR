#pragma once

#include "MaterialAssetReference.h"
#include "MaterialCompiler/MaterialDomain.h"

// A material that shades shape renderables, the project's shape_materials/ folder
class ShapeMaterialAssetReference : public MaterialAssetReference
{
public:
	ShapeMaterialAssetReference()= default;

	inline static const std::string k_assetClassName= "ShapeMaterialAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "ShapeMaterial"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_CUBE; }
};

class ShapeMaterialAssetReferenceFactory
	: public TypedAssetReferenceFactory<ShapeMaterialAssetReference, AssetReferenceConfig>
{
public:
	ShapeMaterialAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "ShapeMaterial"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadShapeMaterialDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1]= {MaterialDomainUtils::k_shapeMaterialFilterPattern};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return locText("assets.shapeMaterialFilterDescription"); }

	virtual bool editorCanCreate() const { return true; }
};
