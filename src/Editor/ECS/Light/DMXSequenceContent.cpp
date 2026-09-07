#include "DMXSequenceContent.h"
#include "Logger.h"
#include "RGBPixelGridComponent.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

// This translation unit owns the editor's copy of the stb decoders, which is
// why it is kept out of the unity build.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace
{
// The JSON spellings, which are not the localization keys the property's
// EnumPropertyMetaData carries
const std::string g_scrollDirectionStrings[(int)eDMXScrollDirection::COUNT]= {
	"left",
	"right",
	"up",
	"down",
};

const uint8_t k_black[3]= {0, 0, 0};

bool readWholeFile(const std::filesystem::path& path, std::vector<uint8_t>& outBytes, std::string& outError)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		outError= "could not open " + path.string();
		return false;
	}

	const std::streamsize size= file.tellg();
	file.seekg(0, std::ios::beg);

	outBytes.resize(static_cast<size_t>(std::max<std::streamsize>(size, 0)));
	if (!outBytes.empty() && !file.read(reinterpret_cast<char*>(outBytes.data()), size))
	{
		outError= "could not read " + path.string();
		return false;
	}

	return true;
}

// Copy one tightly packed RGB block out of a larger tightly packed RGB image
void copyRegion(const uint8_t* source, int sourceWidth, int x, int y, DMXPixelCanvas& outCanvas)
{
	for (int row= 0; row < outCanvas.height; ++row)
	{
		const uint8_t* sourceRow= source + (static_cast<size_t>(y + row) * sourceWidth + x) * 3;
		uint8_t* destRow= outCanvas.rgb.data() + static_cast<size_t>(row) * outCanvas.width * 3;

		memcpy(destRow, sourceRow, static_cast<size_t>(outCanvas.width) * 3);
	}
}
} // namespace

const std::string* k_dmxScrollDirectionStrings= g_scrollDirectionStrings;

// -- DMXPixelCanvas -----
void DMXPixelCanvas::resize(int inWidth, int inHeight, const uint8_t fill[3])
{
	width= std::max(inWidth, 0);
	height= std::max(inHeight, 0);
	rgb.assign(static_cast<size_t>(width) * height * 3, 0);

	if (fill != nullptr && !rgb.empty())
	{
		for (size_t i= 0; i + 2 < rgb.size(); i+= 3)
		{
			rgb[i]= fill[0];
			rgb[i + 1]= fill[1];
			rgb[i + 2]= fill[2];
		}
	}
}

void DMXPixelCanvas::getPixel(int x, int y, const uint8_t fallback[3], uint8_t outRGB[3]) const
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		const uint8_t* source= fallback != nullptr ? fallback : k_black;
		outRGB[0]= source[0];
		outRGB[1]= source[1];
		outRGB[2]= source[2];
		return;
	}

	const size_t offset= (static_cast<size_t>(y) * width + x) * 3;
	outRGB[0]= rgb[offset];
	outRGB[1]= rgb[offset + 1];
	outRGB[2]= rgb[offset + 2];
}

void DMXPixelCanvas::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
		return;

	const size_t offset= (static_cast<size_t>(y) * width + x) * 3;
	rgb[offset]= r;
	rgb[offset + 1]= g;
	rgb[offset + 2]= b;
}

// -- DMXSequenceContent -----
namespace DMXSequenceContent
{
bool loadImage(const std::filesystem::path& path, DMXPixelCanvas& outCanvas, std::string& outError)
{
	outError.clear();

	std::vector<uint8_t> fileBytes;
	if (!readWholeFile(path, fileBytes, outError))
		return false;

	int width= 0, height= 0, sourceChannels= 0;
	stbi_uc* pixels= stbi_load_from_memory(fileBytes.data(), static_cast<int>(fileBytes.size()), &width, &height,
										   &sourceChannels, 3);
	if (pixels == nullptr)
	{
		const char* reason= stbi_failure_reason();
		outError= std::string("could not decode ") + path.string() + ": " + (reason != nullptr ? reason : "unknown");
		return false;
	}

	outCanvas.resize(width, height);
	memcpy(outCanvas.rgb.data(), pixels, outCanvas.rgb.size());
	stbi_image_free(pixels);

	return true;
}

bool sliceSpriteSheet(const DMXPixelCanvas& sheet, int frameWidth, int frameHeight,
					  std::vector<DMXPixelCanvas>& outFrames)
{
	outFrames.clear();

	if (sheet.isEmpty())
		return false;

	// A zero size means square frames the height of the sheet, which is the
	// common single-row strip
	const int width= frameWidth > 0 ? frameWidth : sheet.height;
	const int height= frameHeight > 0 ? frameHeight : sheet.height;
	if (width <= 0 || height <= 0 || width > sheet.width || height > sheet.height)
		return false;

	const int columns= sheet.width / width;
	const int rows= sheet.height / height;
	if (columns <= 0 || rows <= 0)
		return false;

	outFrames.reserve(static_cast<size_t>(columns) * rows);
	for (int row= 0; row < rows; ++row)
	{
		for (int col= 0; col < columns; ++col)
		{
			DMXPixelCanvas frame;
			frame.resize(width, height);
			copyRegion(sheet.rgb.data(), sheet.width, col * width, row * height, frame);
			outFrames.push_back(std::move(frame));
		}
	}

	return true;
}

bool loadAnimation(const std::filesystem::path& path, int spriteFrameWidth, int spriteFrameHeight, float spriteFps,
				   std::vector<DMXPixelCanvas>& outFrames, std::vector<int>& outFrameDelaysMs, std::string& outError)
{
	outFrames.clear();
	outFrameDelaysMs.clear();
	outError.clear();

	std::string extension= path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
				   [](unsigned char c) { return (char)std::tolower(c); });

	if (extension == ".gif")
	{
		std::vector<uint8_t> fileBytes;
		if (!readWholeFile(path, fileBytes, outError))
			return false;

		// One call returns every frame stacked in one buffer plus the file's
		// own per-frame delays in milliseconds
		int* delays= nullptr;
		int width= 0, height= 0, frameCount= 0, sourceChannels= 0;
		stbi_uc* pixels= stbi_load_gif_from_memory(fileBytes.data(), static_cast<int>(fileBytes.size()), &delays,
												   &width, &height, &frameCount, &sourceChannels, 3);
		if (pixels == nullptr)
		{
			const char* reason= stbi_failure_reason();
			outError=
				std::string("could not decode ") + path.string() + ": " + (reason != nullptr ? reason : "unknown");
			return false;
		}

		const size_t frameBytes= static_cast<size_t>(width) * height * 3;
		for (int frameIndex= 0; frameIndex < frameCount; ++frameIndex)
		{
			DMXPixelCanvas frame;
			frame.resize(width, height);
			memcpy(frame.rgb.data(), pixels + static_cast<size_t>(frameIndex) * frameBytes, frameBytes);
			outFrames.push_back(std::move(frame));

			// A GIF that declares no delay is conventionally played at 10fps
			const int delayMs= (delays != nullptr && delays[frameIndex] > 0) ? delays[frameIndex] : 100;
			outFrameDelaysMs.push_back(delayMs);
		}

		stbi_image_free(pixels);
		free(delays);

		if (outFrames.empty())
		{
			outError= path.string() + " decoded no frames";
			return false;
		}

		return true;
	}

	// Any other format is a sprite sheet, timed by the frame rate property
	DMXPixelCanvas sheet;
	if (!loadImage(path, sheet, outError))
		return false;

	if (!sliceSpriteSheet(sheet, spriteFrameWidth, spriteFrameHeight, outFrames))
	{
		outError= path.string() + " could not be split into frames of the requested size";
		return false;
	}

	const float framesPerSecond= spriteFps > 0.f ? spriteFps : 10.f;
	const int delayMs= std::max(1, (int)std::lround(1000.f / framesPerSecond));
	outFrameDelaysMs.assign(outFrames.size(), delayMs);

	return true;
}

void decodeUtf8(const std::string& text, std::vector<uint32_t>& outCodepoints)
{
	outCodepoints.clear();

	size_t index= 0;
	while (index < text.size())
	{
		const uint8_t lead= (uint8_t)text[index];

		int extraBytes= 0;
		uint32_t codepoint= 0;
		if (lead < 0x80)
		{
			codepoint= lead;
		}
		else if ((lead & 0xE0) == 0xC0)
		{
			extraBytes= 1;
			codepoint= lead & 0x1Fu;
		}
		else if ((lead & 0xF0) == 0xE0)
		{
			extraBytes= 2;
			codepoint= lead & 0x0Fu;
		}
		else if ((lead & 0xF8) == 0xF0)
		{
			extraBytes= 3;
			codepoint= lead & 0x07u;
		}
		else
		{
			// A stray continuation or an invalid lead: one replacement, and the
			// scan moves on a byte so malformed input cannot stall it
			outCodepoints.push_back(0xFFFDu);
			index++;
			continue;
		}

		if (index + extraBytes >= text.size())
		{
			outCodepoints.push_back(0xFFFDu);
			index++;
			continue;
		}

		bool bValid= true;
		for (int i= 1; i <= extraBytes; ++i)
		{
			const uint8_t continuation= (uint8_t)text[index + i];
			if ((continuation & 0xC0) != 0x80)
			{
				bValid= false;
				break;
			}
			codepoint= (codepoint << 6) | (continuation & 0x3Fu);
		}

		if (!bValid)
		{
			outCodepoints.push_back(0xFFFDu);
			index++;
			continue;
		}

		outCodepoints.push_back(codepoint);
		index+= static_cast<size_t>(extraBytes) + 1;
	}
}

bool rasterizeText(const std::string& utf8Text, const std::filesystem::path& fontPath, int pixelHeight,
				   const uint8_t foreground[3], const uint8_t background[3], DMXPixelCanvas& outCanvas,
				   std::string& outError)
{
	outError.clear();
	outCanvas= DMXPixelCanvas();

	if (pixelHeight <= 0)
	{
		outError= "text height must be positive";
		return false;
	}

	std::vector<uint8_t> fontBytes;
	if (!readWholeFile(fontPath, fontBytes, outError))
		return false;

	stbtt_fontinfo font;
	if (stbtt_InitFont(&font, fontBytes.data(), stbtt_GetFontOffsetForIndex(fontBytes.data(), 0)) == 0)
	{
		outError= "could not parse font " + fontPath.string();
		return false;
	}

	std::vector<uint32_t> codepoints;
	decodeUtf8(utf8Text, codepoints);
	if (codepoints.empty())
	{
		outError= "no text to rasterize";
		return false;
	}

	// Scale the face so its ascent to descent fits the requested height
	const float scale= stbtt_ScaleForPixelHeight(&font, (float)pixelHeight);
	int ascent= 0, descent= 0, lineGap= 0;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
	const int baseline= (int)std::lround(ascent * scale);

	// Total advance first, so the canvas is allocated once
	float totalAdvance= 0.f;
	for (size_t i= 0; i < codepoints.size(); ++i)
	{
		int advanceWidth= 0, leftSideBearing= 0;
		stbtt_GetCodepointHMetrics(&font, (int)codepoints[i], &advanceWidth, &leftSideBearing);
		totalAdvance+= advanceWidth * scale;

		if (i + 1 < codepoints.size())
			totalAdvance+= stbtt_GetCodepointKernAdvance(&font, (int)codepoints[i], (int)codepoints[i + 1]) * scale;
	}

	const int canvasWidth= std::max(1, (int)std::ceil(totalAdvance));
	outCanvas.resize(canvasWidth, pixelHeight, background);

	float penX= 0.f;
	for (size_t i= 0; i < codepoints.size(); ++i)
	{
		const int codepoint= (int)codepoints[i];

		int glyphWidth= 0, glyphHeight= 0, offsetX= 0, offsetY= 0;
		unsigned char* coverage=
			stbtt_GetCodepointBitmap(&font, 0.f, scale, codepoint, &glyphWidth, &glyphHeight, &offsetX, &offsetY);
		if (coverage != nullptr)
		{
			// The glyph box is relative to the pen on the baseline
			const int originX= (int)std::lround(penX) + offsetX;
			const int originY= baseline + offsetY;

			for (int glyphRow= 0; glyphRow < glyphHeight; ++glyphRow)
			{
				for (int glyphCol= 0; glyphCol < glyphWidth; ++glyphCol)
				{
					const float alpha= coverage[glyphRow * glyphWidth + glyphCol] / 255.f;
					if (alpha <= 0.f)
						continue;

					uint8_t blended[3];
					for (int channel= 0; channel < 3; ++channel)
					{
						const float mixed= background[channel] * (1.f - alpha) + foreground[channel] * alpha;
						blended[channel]= (uint8_t)std::clamp((int)std::lround(mixed), 0, 255);
					}

					outCanvas.setPixel(originX + glyphCol, originY + glyphRow, blended[0], blended[1], blended[2]);
				}
			}

			stbtt_FreeBitmap(coverage, nullptr);
		}

		int advanceWidth= 0, leftSideBearing= 0;
		stbtt_GetCodepointHMetrics(&font, codepoint, &advanceWidth, &leftSideBearing);
		penX+= advanceWidth * scale;

		if (i + 1 < codepoints.size())
			penX+= stbtt_GetCodepointKernAdvance(&font, codepoint, (int)codepoints[i + 1]) * scale;
	}

	return true;
}

float computeScrollPeriod(float pixelsPerSecond, int contentExtent, int gridExtent)
{
	const float travel= (float)(std::max(contentExtent, 0) + std::max(gridExtent, 0));
	if (pixelsPerSecond <= 0.f || travel <= 0.f)
		return 0.f;

	return travel / pixelsPerSecond;
}

float computeScrollOffset(float timeSinceStart, float pixelsPerSecond, eDMXScrollDirection direction, int contentExtent,
						  int gridExtent, bool bLoop)
{
	const float travel= (float)(std::max(contentExtent, 0) + std::max(gridExtent, 0));

	float distance= std::max(timeSinceStart, 0.f) * pixelsPerSecond;
	if (bLoop && travel > 0.f)
	{
		distance= std::fmod(distance, travel);
	}

	// The window starts one grid off the near edge and ends at the content's
	// far edge, so the content fully enters and fully leaves either way
	const bool bReverse= (direction == eDMXScrollDirection::right || direction == eDMXScrollDirection::down);

	return bReverse ? (float)contentExtent - distance : distance - (float)gridExtent;
}

float computeAnimationPeriod(const std::vector<int>& frameDelaysMs, float speedScale)
{
	const float scale= speedScale > 0.f ? speedScale : 1.f;

	int totalMs= 0;
	for (const int delayMs : frameDelaysMs)
		totalMs+= std::max(delayMs, 0);

	return (totalMs / 1000.f) / scale;
}

int computeAnimationFrame(float timeSinceStart, const std::vector<int>& frameDelaysMs, float speedScale, bool bLoop)
{
	if (frameDelaysMs.empty())
		return -1;

	const float period= computeAnimationPeriod(frameDelaysMs, speedScale);
	if (period <= 0.f)
		return 0;

	float time= std::max(timeSinceStart, 0.f);
	if (bLoop)
	{
		time= std::fmod(time, period);
	}
	else if (time >= period)
	{
		// A finished one-shot holds its last frame
		return (int)frameDelaysMs.size() - 1;
	}

	const float scale= speedScale > 0.f ? speedScale : 1.f;
	float elapsed= 0.f;
	for (size_t frameIndex= 0; frameIndex < frameDelaysMs.size(); ++frameIndex)
	{
		elapsed+= (std::max(frameDelaysMs[frameIndex], 0) / 1000.f) / scale;
		if (time < elapsed)
			return (int)frameIndex;
	}

	return (int)frameDelaysMs.size() - 1;
}

bool writeGridPixel(const RGBPixelGridDefinition& gridDefinition, int col, int row, uint8_t r, uint8_t g, uint8_t b,
					std::vector<uint8_t>& inoutValues)
{
	// The grid's origin corner and zig-zag wiring decide where a cell lands in
	// the DMX stream, and this is the only place a content source interprets it
	const int wireIndex= gridDefinition.getPixelWireIndex(col, row);
	if (wireIndex < 0)
		return false;

	const size_t channelCount= static_cast<size_t>(gridDefinition.getTotalChannels());
	if (inoutValues.size() < channelCount)
		inoutValues.resize(channelCount, 0);

	const size_t offset= static_cast<size_t>(wireIndex) * 3;
	if (offset + 3 > inoutValues.size())
		return false;

	inoutValues[offset]= r;
	inoutValues[offset + 1]= g;
	inoutValues[offset + 2]= b;

	return true;
}

void blitWindow(const DMXPixelCanvas& canvas, int srcX, int srcY, const RGBPixelGridDefinition& gridDefinition,
				const uint8_t background[3], std::vector<uint8_t>& outValues)
{
	const int columns= gridDefinition.getColumns();
	const int rows= gridDefinition.getRows();

	outValues.assign(static_cast<size_t>(gridDefinition.getTotalChannels()), 0);

	for (int row= 0; row < rows; ++row)
	{
		for (int col= 0; col < columns; ++col)
		{
			uint8_t pixel[3];
			canvas.getPixel(srcX + col, srcY + row, background, pixel);

			writeGridPixel(gridDefinition, col, row, pixel[0], pixel[1], pixel[2], outValues);
		}
	}
}
} // namespace DMXSequenceContent
