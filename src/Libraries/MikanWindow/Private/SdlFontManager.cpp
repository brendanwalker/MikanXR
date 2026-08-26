#include "SdlFontManager.h"
#include "IMkTexture.h"
#include "IMkFontManager.h"
#include "Logger.h"
#include "PathUtils.h"
#include "StringUtils.h"

#include "SDL_ttf.h"

#include <cstring>
#include <unordered_map>
#include <vector>

SdlFontManager::SdlFontManager() {}

SdlFontManager::~SdlFontManager() {}

bool SdlFontManager::startup()
{
	if (TTF_WasInit() == 0)
	{
		if (TTF_Init() == -1)
		{
			MIKAN_LOG_ERROR("FontManager::startup") << "FontManager failed to initialize SDL TTF lib.";
			return false;
		}
	}

	return true;
}

void SdlFontManager::garbageCollect()
{
	for (auto it= m_bakedTextCache.begin(); it != m_bakedTextCache.end();)
	{
		SdlFontManager::MkBakedText& bakedText= it->second;

		// Age the baked text
		--bakedText.lifetime;

		// Kill any baked text whose lifetime has expired
		if (bakedText.lifetime <= 0)
		{
			bakedText.texture= nullptr;

			it= m_bakedTextCache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void SdlFontManager::shutdown()
{
	// Flush any remaining baked textures
	m_bakedTextCache.clear();

	// Flush any loaded fonts
	for (auto it= m_ttfFontCache.begin(); it != m_ttfFontCache.end(); ++it)
	{
		TTF_Font* font= (TTF_Font*)it->second;
		TTF_CloseFont(font);
	}
	m_ttfFontCache.clear();
}

size_t computeTextHash(const TextStyle& style, const std::wstring& text)
{
	std::hash<std::wstring> hasher;

	wchar_t szStyleString[256];
	StringUtils::formatWString(
		szStyleString, sizeof(szStyleString), L"%s_%d_%d_%d_%d_%d_%d_%d_%d_%d", style.fontName.c_str(), style.pointSize,
		style.styleBitmask, (int)style.hasShadow, (int)(style.shadowColor.r * 255), (int)(style.shadowColor.g * 255),
		(int)(style.shadowColor.b * 255), style.shadowOffset.x, style.shadowOffset.y, (int)(style.shadowOpacity * 255));

	return hasher(text + szStyleString);
}

IMkTexturePtr SdlFontManager::fetchBakedText(const TextStyle& style, const std::wstring& text)
{
	const int defaultLifetime= 10;
	const size_t hash= computeTextHash(style, text);

	if (m_bakedTextCache.find(hash) != m_bakedTextCache.end())
	{
		MkBakedText& bakedText= m_bakedTextCache[hash];
		bakedText.lifetime= defaultLifetime;
		return bakedText.texture;
	}

	TTF_Font* font= (TTF_Font*)fetchFont(style.fontName, style.pointSize);
	if (font == nullptr)
		return nullptr;

	// Apply bold/italic/underline/strikethrough to the font before rendering
	TTF_SetFontStyle(font, (int)style.styleBitmask);

	typedef std::basic_string<Uint16, std::char_traits<Uint16>, std::allocator<Uint16>> u16string;
	const u16string utext(text.begin(), text.end());

	SDL_Color fgColor= {(Uint8)(style.color.r * 255.f), (Uint8)(style.color.g * 255.f), (Uint8)(style.color.b * 255.f),
						255};

	SDL_Surface* textSurface= TTF_RenderUNICODE_Blended(font, utext.c_str(), fgColor);
	if (textSurface == nullptr)
		return nullptr;

	// Composite shadow + text into a single surface when a shadow is requested
	SDL_Surface* finalSurface= nullptr;
	if (style.hasShadow)
	{
		SDL_Color shadowSdlColor= {(Uint8)(style.shadowColor.r * 255.f), (Uint8)(style.shadowColor.g * 255.f),
								   (Uint8)(style.shadowColor.b * 255.f), 255};

		SDL_Surface* shadowSurface= TTF_RenderUNICODE_Blended(font, utext.c_str(), shadowSdlColor);
		if (shadowSurface != nullptr)
		{
			const int ox= style.shadowOffset.x;
			const int oy= style.shadowOffset.y;
			const int combinedW= textSurface->w + std::abs(ox);
			const int combinedH= textSurface->h + std::abs(oy);

			finalSurface= SDL_CreateRGBSurface(0, combinedW, combinedH, textSurface->format->BitsPerPixel,
											   textSurface->format->Rmask, textSurface->format->Gmask,
											   textSurface->format->Bmask, textSurface->format->Amask);

			if (finalSurface != nullptr)
			{
				SDL_FillRect(finalSurface, nullptr, SDL_MapRGBA(finalSurface->format, 0, 0, 0, 0));

				// Blit shadow at offset, with reduced opacity
				SDL_SetSurfaceAlphaMod(shadowSurface, (Uint8)(style.shadowOpacity * 255.f));
				SDL_SetSurfaceBlendMode(shadowSurface, SDL_BLENDMODE_BLEND);
				SDL_Rect shadowDst= {ox >= 0 ? ox : 0, oy >= 0 ? oy : 0, 0, 0};
				SDL_BlitSurface(shadowSurface, nullptr, finalSurface, &shadowDst);

				// Blit main text on top
				SDL_SetSurfaceBlendMode(textSurface, SDL_BLENDMODE_BLEND);
				SDL_Rect textDst= {ox >= 0 ? 0 : -ox, oy >= 0 ? 0 : -oy, 0, 0};
				SDL_BlitSurface(textSurface, nullptr, finalSurface, &textDst);
			}

			SDL_FreeSurface(shadowSurface);
		}
	}

	// Fall back to plain text surface if shadow compositing failed or wasn't requested
	if (finalSurface == nullptr)
		finalSurface= textSurface;

	// SDL_ttf can hand back surfaces with row padding (pitch > w*4), but the
	// texture upload assumes tightly packed rows, so repack when they differ
	const int packedRowSize= finalSurface->w * 4;
	const uint8_t* pixelData= (const uint8_t*)finalSurface->pixels;
	std::vector<uint8_t> packedPixels;
	if (finalSurface->pitch != packedRowSize)
	{
		packedPixels.resize((size_t)packedRowSize * finalSurface->h);
		for (int y= 0; y < finalSurface->h; ++y)
		{
			std::memcpy(&packedPixels[(size_t)y * packedRowSize],
						(const uint8_t*)finalSurface->pixels + (size_t)y * finalSurface->pitch, packedRowSize);
		}
		pixelData= packedPixels.data();
	}

	IMkTexturePtr result;
	IMkTexturePtr texture=
		CreateMkTexture((uint16_t)finalSurface->w, (uint16_t)finalSurface->h, pixelData, MK_RGBA, MK_BGRA);

	if (texture->createTexture())
	{
		MkBakedText bakedText= {texture, text, defaultLifetime};
		m_bakedTextCache.insert({hash, bakedText});
		result= texture;
	}

	if (finalSurface != textSurface)
		SDL_FreeSurface(finalSurface);
	SDL_FreeSurface(textSurface);

	return result;
}

size_t computeFontHash(const std::string& fontName, int pointSize)
{
	std::hash<std::string> hasher;

	char szStyleString[256];
	StringUtils::formatString(szStyleString, sizeof(szStyleString), "%s_%d", fontName.c_str(), pointSize);

	return hasher(szStyleString);
}

void* SdlFontManager::fetchFont(const std::string& fontName, int pointSize)
{
	const size_t hash= computeFontHash(fontName, pointSize);

	if (m_ttfFontCache.find(hash) != m_ttfFontCache.end())
	{
		return m_ttfFontCache[hash];
	}
	else
	{
		const std::filesystem::path fontPath= PathUtils::getFontPath(fontName);
		const std::string fontPathString= fontPath.string();

		TTF_Font* font= TTF_OpenFont(fontPathString.c_str(), pointSize);
		if (font != nullptr)
		{
			m_ttfFontCache.insert({hash, font});
			return font;
		}
		else
		{
			MIKAN_LOG_ERROR("FontManager::fetchFont") << "Failed to find font path: " << fontPathString;
			return nullptr;
		}
	}
}

IMkFontManagerPtr createMkFontManager() { return std::make_shared<SdlFontManager>(); }
