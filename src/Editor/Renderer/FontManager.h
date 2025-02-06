#pragma once

#include "TextStyle.h"
#include "MikanRendererFwd.h"

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

	IMkTexturePtr fetchBakedText(const TextStyle& style, const std::wstring& text);

private:
	void* fetchFont(const std::string& fontName, int pointSize);

	struct GlBakedText
	{
		IMkTexturePtr texture;
		std::wstring text;
		int lifetime;
	};

	std::map<size_t, GlBakedText> m_bakedTextCache;
	std::map<size_t, void*> m_ttfFontCache;
};