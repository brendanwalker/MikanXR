#pragma once

#include "AssetReference.h"
#include "LocText.h"

// An image a DMX sequence rasterizes onto a pixel grid. The same reference
// covers a still bitmap and an animation, since an animation is either a GIF
// or a sprite sheet in one of the still formats.
class PixelContentAssetReference : public AssetReference
{
public:
	PixelContentAssetReference()= default;

	inline static const std::string k_assetClassName= "PixelContentAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "PixelContent"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_PICTURE_O; }
};

class PixelContentAssetReferenceFactory
	: public TypedAssetReferenceFactory<PixelContentAssetReference, AssetReferenceConfig>
{
public:
	PixelContentAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "PixelContent"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadPixelContentDialogTitle"); }
	virtual char const* const* getFilterPatterns() const { return getPixelContentFilterPatterns(); }
	virtual int getFilterPatternCount() const { return getPixelContentFilterPatternCount(); }
	virtual char const* getFilterDescription() const { return locText("assets.pixelContentFilterDescription"); }

	static char const* const* getPixelContentFilterPatterns()
	{
		static const char* filterItems[6]= {"*.png", "*.gif", "*.bmp", "*.tga", "*.jpg", "*.jpeg"};
		return filterItems;
	}
	static int getPixelContentFilterPatternCount() { return 6; }
};
