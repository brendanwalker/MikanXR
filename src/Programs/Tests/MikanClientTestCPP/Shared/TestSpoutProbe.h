#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Read a Spout sender back to CPU pixels (tightly packed RGBA8, top row first) through the same
// spoutDX receiver path the editor's reader uses, so a headless drive can check what a client's
// shared texture actually carries rather than what a compositor graph shows. Waits briefly for
// the sender to answer a frame.
bool readSpoutSenderRgba(const std::string& senderName, std::vector<uint8_t>& outRgbaPixels, int& outWidth,
						 int& outHeight);
