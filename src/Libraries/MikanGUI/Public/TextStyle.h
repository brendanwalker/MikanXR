#pragma once

#include "IMkTextRenderer.h"

#include <filesystem>
#include <string>

const TextStyle& getDefaultTextStyle();
const std::filesystem::path getDefaultJapaneseFontPath();
const std::filesystem::path getForkAwesomeWebFontPath();

