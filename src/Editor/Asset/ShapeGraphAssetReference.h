#pragma once

#include "AssetReference.h"
#include "Graphs/NodeGraphFileTypes.h"
#include "LocText.h"

// A shape node graph file, the project's shapes/ folder
class ShapeGraphAssetReference : public AssetReference
{
public:
	ShapeGraphAssetReference()= default;

	inline static const std::string k_assetClassName= "ShapeGraphAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "ShapeGraph"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_CUBES; }
};

class ShapeGraphAssetReferenceFactory
	: public TypedAssetReferenceFactory<ShapeGraphAssetReference, AssetReferenceConfig>
{
public:
	ShapeGraphAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "ShapeGraph"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadShapeGraphDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[1]= {NodeGraphFileTypes::k_shapeGraphFilterPattern};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 1; }
	virtual char const* getFilterDescription() const { return locText("assets.shapeGraphFilterDescription"); }
};
