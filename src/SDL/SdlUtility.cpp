#include "GlTexture.h"
#include "GlCommon.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

namespace SdlUtility
{
	bool saveTextureToPNG(GlTexture* texture, const char* filename)
	{
		int depth = 0;
		switch (texture->getBufferFormat())
		{
			case GL_RGB:
				depth = 24;
				break;
			case GL_RGBA:
				depth = 32;
				break;
			default:
				break;
		}

		if (texture->getTextureWidth() == 0 || texture->getTextureHeight() == 0 || depth < 24)
		{
			return false;
		}

		Uint32 rmask, gmask, bmask, amask;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		int shift = (depth == 24) ? 8 : 0;
		rmask = 0xff000000 >> shift;
		gmask = 0x00ff0000 >> shift;
		bmask = 0x0000ff00 >> shift;
		amask = 0x000000ff >> shift;
	#else // little endian, like x86
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = (depth == 24) ? 0 : 0xff000000;
	#endif

		const int bytesPerPixel = depth / 8;
		const int pitch = texture->getTextureWidth() * bytesPerPixel;
		uint8_t* buffer = new uint8_t[pitch * texture->getTextureHeight()];

		texture->copyTextureIntoBuffer(buffer);

		SDL_Surface* surface =
			SDL_CreateRGBSurfaceFrom(
				(void*)buffer,
				texture->getTextureWidth(), texture->getTextureHeight(),
				32, pitch,
				rmask, gmask, bmask, amask);

		bool bSuccess = false;
		if (surface != nullptr)
		{
			bSuccess = IMG_SavePNG(surface, filename) == 0;
			SDL_FreeSurface(surface);
		}

		delete[] buffer;

		return bSuccess;
	}
}