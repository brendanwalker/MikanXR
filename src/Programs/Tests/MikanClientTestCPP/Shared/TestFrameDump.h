#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Write tightly packed RGBA8 pixels, top row first, as a PNG
bool writeRgbaPng(const std::string& path, const std::vector<uint8_t>& rgbaPixels, int width, int height);
