#pragma once

#include "TextStyle.h"
#include "RendererFwd.h"

#include <string>
#include <map>

#include <stdint.h>

#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Types.h>

class FontManager
{
public:
	FontManager();
	virtual ~FontManager();

	bool startup();
	void garbageCollect();
	void shutdown();

	GlTexturePtr fetchBakedText(const TextStyle& style, const std::wstring& text);

private:
	void* fetchFont(const std::string& fontName, int pointSize);

	struct GlBakedText
	{
		GlTexturePtr texture;
		std::wstring text;
		int lifetime;
	};

	std::map<size_t, GlBakedText> m_bakedTextCache;
	std::map<size_t, void*> m_ttfFontCache;
};