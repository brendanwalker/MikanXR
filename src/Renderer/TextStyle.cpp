#include "Colors.h"
#include "TextStyle.h"
#include "PathUtils.h"

static TextStyle gDefaultTextStyle = {
	"mona",
	18,
	TEXT_STYLE_NORMAL,
	eHorizontalTextAlignment::Middle,
	eVerticalTextAlignment::Middle,
	Colors::GhostWhite
};

const TextStyle& getDefaultTextStyle()
{
	return gDefaultTextStyle;
}

const std::filesystem::path getDefaultJapaneseFontPath()
{
	const std::filesystem::path japaneseFontPath= getFontPath("mona");

	return japaneseFontPath;
}

const std::filesystem::path getForkAwesomeWebFontPath()
{
	const std::filesystem::path fontPath = getFontPath("forkawesome-webfont");

	return fontPath;
}

const std::filesystem::path getFontPath(const std::string& fontName)
{
	const std::string fileFilename= fontName + ".ttf";

	return PathUtils::getResourceDirectory() / "font" / fileFilename;
}