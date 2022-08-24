#pragma once

#include "TextStyle.h"

#include <string>
#include <map>

#include <stdint.h>

class GlBakedTextCache
{
public:
	GlBakedTextCache();
	virtual ~GlBakedTextCache();

	bool startup();
	void garbageCollect();
	void shutdown();

	class GlTexture* fetchBakedText(const TextStyle& style, const std::wstring& text);

private:
	void* fetchFont(const std::string& fontName, int pointSize);

	struct GlBakedText
	{
		class GlTexture* texture;
		int lifetime;
	};

	std::map<size_t, GlBakedText> m_bakedTextCache;
	std::map<size_t, void*> m_ttfFontCache;
};