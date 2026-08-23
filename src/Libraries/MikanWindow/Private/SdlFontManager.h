#pragma once

#include "IMkTextRenderer.h"

#include <string>
#include <map>

#include <stdint.h>

class SdlFontManager : public IMkFontManager
{
public:
	SdlFontManager();
	virtual ~SdlFontManager();

	virtual bool startup() override;
	virtual void garbageCollect() override;
	virtual void shutdown() override;

	virtual IMkTexturePtr fetchBakedText(const TextStyle& style, const std::wstring& text) override;

private:
	void* fetchFont(const std::string& fontName, int pointSize);

	struct MkBakedText
	{
		IMkTexturePtr texture;
		std::wstring text;
		int lifetime;
	};

	std::map<size_t, MkBakedText> m_bakedTextCache;
	std::map<size_t, void*> m_ttfFontCache;
};
