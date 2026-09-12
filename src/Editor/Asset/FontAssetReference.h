#pragma once

#include "AssetReference.h"
#include "LocText.h"

// A TrueType face a DMX sequence rasterizes text with. Unlike the GUI fonts,
// which the ImGui atlas owns, this one is read on the CPU a glyph at a time.
class FontAssetReference : public AssetReference
{
public:
	FontAssetReference()= default;

	inline static const std::string k_assetClassName= "FontAssetReference";
	virtual std::string getClassName() const override { return k_assetClassName; }
	virtual std::string getAssetTypeName() const override { return "Font"; }
	virtual const char* editorGetIcon() const override { return ICON_FK_FONT; }
};

class FontAssetReferenceFactory : public TypedAssetReferenceFactory<FontAssetReference, AssetReferenceConfig>
{
public:
	FontAssetReferenceFactory();

	virtual std::string getAssetTypeName() const { return "Font"; }
	virtual char const* getFileDialogTitle() const { return locText("assets.loadFontDialogTitle"); }
	virtual char const* const* getFilterPatterns() const
	{
		static const char* filterItems[2]= {"*.ttf", "*.otf"};
		return filterItems;
	}
	virtual int getFilterPatternCount() const { return 2; }
	virtual char const* getFilterDescription() const { return locText("assets.fontFilterDescription"); }

	// The bundled face the text sources default to, chosen because it carries
	// kana and kanji as well as Latin. In stored form: resolve it through
	// PathUtils::resolveProjectResource before opening it.
	static std::filesystem::path getDefaultFontPath();
};
