#pragma once

#include <filesystem>
#include <string>

#include "glm/ext/vector_float3.hpp"

#include "IMkTextRenderer.h"

const TextStyle& getDefaultTextStyle();
const std::filesystem::path getDefaultJapaneseFontPath();
const std::filesystem::path getForkAwesomeWebFontPath();
const std::filesystem::path getFontPath(const std::string& fontName);

