#pragma once

#include "MkGuiExport.h"
#include "IMkTextRenderer.h"

#include <filesystem>
#include <string>

MIKAN_GUI_FUNC(const TextStyle&) getDefaultTextStyle();
MIKAN_GUI_FUNC(const std::filesystem::path) getDefaultJapaneseFontPath();
MIKAN_GUI_FUNC(const std::filesystem::path) getForkAwesomeWebFontPath();
