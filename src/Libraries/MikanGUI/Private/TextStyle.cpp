#include "TextStyle.h"
#include "PathUtils.h"

static TextStyle gDefaultTextStyle= {
	"mona",
	18,
	TEXT_STYLE_NORMAL,
	eHorizontalTextAlignment::Middle,
	eVerticalTextAlignment::Middle,
	{0.972549f, 0.972549f, 1.000000f},
	false,           // hasShadow
	{0.f, 0.f, 0.f}, // shadowColor
	{2, 2},          // shadowOffset
	0.7f             // shadowOpacity
};

const TextStyle& getDefaultTextStyle() { return gDefaultTextStyle; }

const std::filesystem::path getDefaultJapaneseFontPath()
{
	const std::filesystem::path japaneseFontPath= PathUtils::getFontPath("mona");

	return japaneseFontPath;
}

const std::filesystem::path getForkAwesomeWebFontPath()
{
	const std::filesystem::path fontPath= PathUtils::getFontPath("forkawesome-webfont");

	return fontPath;
}