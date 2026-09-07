#include "TestFrameDump.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

bool writeRgbaPng(const std::string& path, const std::vector<uint8_t>& rgbaPixels, int width, int height)
{
	if (width <= 0 || height <= 0 || rgbaPixels.size() < (size_t)width * (size_t)height * 4)
		return false;

	return stbi_write_png(path.c_str(), width, height, 4, rgbaPixels.data(), width * 4) != 0;
}
