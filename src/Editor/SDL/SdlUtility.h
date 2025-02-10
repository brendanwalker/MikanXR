#pragma once

#include <memory>

class IMkTexture;
typedef std::shared_ptr<IMkTexture> IMkTexturePtr;

namespace SdlUtility
{
	bool saveTextureToPNG(IMkTexturePtr texture, const char* filename);
};