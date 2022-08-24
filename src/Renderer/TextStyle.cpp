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

const std::string& getDefaultJapaneseFontPath()
{
	static std::string japaneseFontPath= getFontPath("mona");

	return japaneseFontPath;
}

std::string getFontPath(const std::string& fontName)
{
	return PathUtils::getResourceDirectory() + "\\font\\" + fontName + ".ttf";
}