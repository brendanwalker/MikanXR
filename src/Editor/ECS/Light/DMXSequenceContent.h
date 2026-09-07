#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class RGBPixelGridDefinition;

// Which way the content travels across the grid. The window moves the opposite
// way through the content, so "left" is the marquee that reads right to left.
enum class eDMXScrollDirection : int
{
	INVALID= -1,

	left= 0,
	right,
	up,
	down,

	COUNT
};
extern const std::string* k_dmxScrollDirectionStrings;

// A small RGB8 image a sequence content source rasterizes into, and a pixel
// grid samples a window of. Row major, three bytes per pixel.
struct DMXPixelCanvas
{
	int width= 0;
	int height= 0;
	std::vector<uint8_t> rgb;

	inline bool isEmpty() const { return width <= 0 || height <= 0; }
	void resize(int inWidth, int inHeight, const uint8_t fill[3]= nullptr);
	// Out of range reads answer the fallback rather than clamping to an edge,
	// which is what makes a scrolled window blank outside the content
	void getPixel(int x, int y, const uint8_t fallback[3], uint8_t outRGB[3]) const;
	void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
};

namespace DMXSequenceContent
{
// One still image, forced to three channels
bool loadImage(const std::filesystem::path& path, DMXPixelCanvas& outCanvas, std::string& outError);

// A .gif carries its own per-frame delays. Any other extension is read as a
// sprite sheet and every frame gets the same delay from spriteFps. A zero
// sprite frame size means square frames the height of the sheet.
bool loadAnimation(const std::filesystem::path& path, int spriteFrameWidth, int spriteFrameHeight, float spriteFps,
				   std::vector<DMXPixelCanvas>& outFrames, std::vector<int>& outFrameDelaysMs, std::string& outError);

// Split a sheet into frames left to right, then top to bottom. A frame size
// that does not divide the sheet leaves the remainder out.
bool sliceSpriteSheet(const DMXPixelCanvas& sheet, int frameWidth, int frameHeight,
					  std::vector<DMXPixelCanvas>& outFrames);

// UTF-8 to codepoints. A malformed or truncated sequence contributes one
// U+FFFD and the scan resumes at the next byte, so bad input never loops.
void decodeUtf8(const std::string& text, std::vector<uint32_t>& outCodepoints);

// One line of text at pixelHeight, glyph coverage blended between the two
// colors. The canvas is exactly pixelHeight tall and as wide as the advances.
bool rasterizeText(const std::string& utf8Text, const std::filesystem::path& fontPath, int pixelHeight,
				   const uint8_t foreground[3], const uint8_t background[3], DMXPixelCanvas& outCanvas,
				   std::string& outError);

// Where the grid's window sits in the content at this time, along the scroll
// axis. The sweep runs from one grid-width off the near edge to the far edge of
// the content, so the content fully enters and fully leaves. Looping wraps the
// distance over that same sweep, which puts a blank gap between repeats.
float computeScrollOffset(float timeSinceStart, float pixelsPerSecond, eDMXScrollDirection direction, int contentExtent,
						  int gridExtent, bool bLoop);
// Seconds for one full sweep, zero when the speed or the extent is zero
float computeScrollPeriod(float pixelsPerSecond, int contentExtent, int gridExtent);

// Frame index at this time, or -1 with no frames. speedScale above one plays
// faster. Not looping holds the last frame.
int computeAnimationFrame(float timeSinceStart, const std::vector<int>& frameDelaysMs, float speedScale, bool bLoop);
// Seconds for one pass through every frame
float computeAnimationPeriod(const std::vector<int>& frameDelaysMs, float speedScale);

// Copy a columns x rows window of the canvas into a fixture value buffer, each
// cell landing at its own wire position. Cells outside the canvas take the
// background. The buffer grows to the grid's channel count.
void blitWindow(const DMXPixelCanvas& canvas, int srcX, int srcY, const RGBPixelGridDefinition& gridDefinition,
				const uint8_t background[3], std::vector<uint8_t>& outValues);

// One grid cell's color at its wire position, growing the buffer to the grid's
// channel count. False when the cell is outside the grid.
bool writeGridPixel(const RGBPixelGridDefinition& gridDefinition, int col, int row, uint8_t r, uint8_t g, uint8_t b,
					std::vector<uint8_t>& inoutValues);
} // namespace DMXSequenceContent
