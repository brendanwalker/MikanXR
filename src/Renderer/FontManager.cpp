#include "FontManager.h"
#include "GlCommon.h"
#include "GlShaderCache.h"
#include "GlStaticMeshInstance.h"
#include "GlProgram.h"
#include "GlTexture.h"
#include "Logger.h"
#include "PathUtils.h"
#include "StringUtils.h"

#include "SDL_ttf.h"

#include <unordered_map>

#include <easy/profiler.h>

#include <RmlUi/Core/Core.h>

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

bool FontManager::startup()
{
	EASY_FUNCTION();

	if (TTF_WasInit() == 0)
	{
		if (TTF_Init() == -1)
		{
			MIKAN_LOG_ERROR("FontManager::startup") << "FontManager failed to initialize SDL TTF lib.";
			return false;
		}
	}

	Rml::SetFontEngineInterface(this);

	return true;
}

void FontManager::garbageCollect()
{
	EASY_FUNCTION();

	for (auto it = m_bakedTextCache.begin(); it != m_bakedTextCache.end(); )
	{
		FontManager::GlBakedText& bakedText = it->second;

		// Age the baked text
		--bakedText.lifetime;

		// Kill any baked text whose lifetime has expired
		if (bakedText.lifetime <= 0)
		{
			bakedText.texture->disposeTexture();
			delete bakedText.texture;

			it= m_bakedTextCache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void FontManager::shutdown()
{	
	// Flush any remaining baked textures
	for (auto it = m_bakedTextCache.begin(); it != m_bakedTextCache.end(); ++it)
	{
		const FontManager::GlBakedText& bakedText= it->second;

		bakedText.texture->disposeTexture();
		delete bakedText.texture;
	}
	m_bakedTextCache.clear();

	// Flush any loaded fonts
	for (auto it = m_ttfFontCache.begin(); it != m_ttfFontCache.end(); ++it)
	{
		TTF_Font* font= (TTF_Font *)it->second;
		TTF_CloseFont(font);
	}
	m_ttfFontCache.clear();
}

size_t computeTextHash(const TextStyle& style, const std::wstring& text)
{
	std::hash<std::wstring> hasher;

	wchar_t szStyleString[256];
	StringUtils::formatWString(szStyleString, sizeof(szStyleString), L"%s_%d_%d",
		style.fontName.c_str(),
		style.pointSize,
		style.styleBitmask);
	
	return hasher(text + szStyleString);
}

GlTexture* FontManager::fetchBakedText(
	const TextStyle& style, 
	const std::wstring& text)
{
	const int defaultLifetime= 10;
	const size_t hash= computeTextHash(style, text);

	if (m_bakedTextCache.find(hash) != m_bakedTextCache.end())
	{
		GlBakedText& bakedText= m_bakedTextCache[hash];

		// Reset the cache lifetime if we just requested the texture again
		bakedText.lifetime= defaultLifetime;
		return bakedText.texture;
	}
	else
	{
		TTF_Font* font = (TTF_Font*)fetchFont(style.fontName, style.pointSize);
		if (font != nullptr)
		{
			typedef std::basic_string<Uint16, std::char_traits<Uint16>, std::allocator<Uint16> > u16string;
			u16string utext_sf_str(text.begin(), text.end());

			SDL_Color sdlColor = { 
				(Uint8)(style.color.r * 255.f),
				(Uint8)(style.color.g * 255.f),
				(Uint8)(style.color.b * 255.f), 255 };
			SDL_Surface* sdlSurface = TTF_RenderUNICODE_Blended(font, utext_sf_str.c_str(), sdlColor);

			if (sdlSurface != nullptr)
			{
				GlTexture* texture =
					new GlTexture(
						(uint16_t)sdlSurface->w, (uint16_t)sdlSurface->h,
						(const uint8_t *)sdlSurface->pixels,
						GL_RGBA, GL_BGRA);

				if (texture->createTexture())
				{
					GlBakedText bakedText = { texture, defaultLifetime };
					m_bakedTextCache.insert({ hash, bakedText });
				}
				else
				{
					delete texture;
					texture= nullptr;
				}

				SDL_FreeSurface(sdlSurface);
				sdlSurface = nullptr;

				return texture;
			}
		}
	}

	return nullptr;
}

size_t computeFontHash(const std::string& fontName, int pointSize)
{
	std::hash<std::string> hasher;

	char szStyleString[256];
	StringUtils::formatString(szStyleString, sizeof(szStyleString), "%s_%d",
		fontName.c_str(),
		pointSize);

	return hasher(szStyleString);
}

void* FontManager::fetchFont(const std::string& fontName, int pointSize)
{
	const size_t hash = computeFontHash(fontName, pointSize);

	if (m_ttfFontCache.find(hash) != m_ttfFontCache.end())
	{
		return m_ttfFontCache[hash];
	}
	else
	{
		const std::string fontPath= getFontPath(fontName);

		TTF_Font* font= TTF_OpenFont(fontPath.c_str(), pointSize);
		if (font != nullptr)
		{
			m_ttfFontCache.insert({hash, font});
			return font;
		}
		else
		{
			MIKAN_LOG_ERROR("FontManager::fetchFont") << "Failed to find font path: " << fontPath;
			return nullptr;
		}
	}
}

// -- Rml::FontEngineInterface -----
bool FontManager::LoadFontFace(
	const Rml::String& file_path,
	bool fallback_face,
	Rml::Style::FontWeight weight)
{
	//TTF_Font* font = (TTF_Font*)fetchFont(style.fontName, style.pointSize);
	return false;
}

bool FontManager::LoadFontFace(
	const Rml::byte* data, int data_size,
	const Rml::String& font_family, Rml::Style::FontStyle style,
	Rml::Style::FontWeight weight, bool fallback_face)
{
	//SDL_RWops* memReader = SDL_RWFromConstMem(data, data_size);
	//TTF_Font* font = TTF_OpenFontRW(memReader, 0, weight);
	//SDL_RWclose(SDL_RWops * context);

	return false;
}

Rml::FontFaceHandle FontManager::GetFontFaceHandle(
	const Rml::String& family,
	Rml::Style::FontStyle style,
	Rml::Style::FontWeight weight,
	int size)
{
	return 0;
}

Rml::FontEffectsHandle FontManager::PrepareFontEffects(
	Rml::FontFaceHandle handle,
	const Rml::FontEffectList& font_effects)
{
	return 0;
}

int FontManager::GetSize(Rml::FontFaceHandle handle)
{
	return 0;
}

int FontManager::GetXHeight(Rml::FontFaceHandle handle)
{
	return 0;
}

int FontManager::GetLineHeight(Rml::FontFaceHandle handle)
{
	return 0;
}

int FontManager::GetBaseline(Rml::FontFaceHandle handle)
{
	return 0;
}

float FontManager::GetUnderline(Rml::FontFaceHandle handle, float& thickness)
{
	return 0;
}

int FontManager::GetStringWidth(
	Rml::FontFaceHandle handle,
	const Rml::String& string,
	Rml::Character prior_character)
{
	return 0;
}

int FontManager::GenerateString(
	Rml::FontFaceHandle face_handle,
	Rml::FontEffectsHandle font_effects_handle,
	const Rml::String& string,
	const Rml::Vector2f& position,
	const Rml::Colourb& colour,
	float opacity,
	Rml::GeometryList& geometry)
{
	return 0;
}

int FontManager::GetVersion(Rml::FontFaceHandle handle)
{
	return 0;
}

void FontManager::ReleaseFontResources()
{
}