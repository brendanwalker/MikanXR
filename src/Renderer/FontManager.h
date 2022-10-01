#pragma once

#include "TextStyle.h"

#include <string>
#include <map>

#include <stdint.h>

#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Types.h>

class FontManager : public Rml::FontEngineInterface
{
public:
	FontManager();
	virtual ~FontManager();

	bool startup();
	void garbageCollect();
	void shutdown();

	class GlTexture* fetchBakedText(const TextStyle& style, const std::wstring& text);

	// -- Rml::FontEngineInterface -----
	virtual bool LoadFontFace(const Rml::String& file_name, bool fallback_face, Rml::Style::FontWeight weight) override;
	virtual bool LoadFontFace(
		const Rml::byte* data, int data_size,
		const Rml::String& family,
		Rml::Style::FontStyle style,
		Rml::Style::FontWeight weight,
		bool fallback_face) override;
	virtual Rml::FontFaceHandle GetFontFaceHandle(
		const Rml::String& family,
		Rml::Style::FontStyle style,
		Rml::Style::FontWeight weight,
		int size) override;
	virtual Rml::FontEffectsHandle PrepareFontEffects(
		Rml::FontFaceHandle handle,
		const Rml::FontEffectList& font_effects) override;
	virtual int GetSize(Rml::FontFaceHandle handle) override;
	virtual int GetXHeight(Rml::FontFaceHandle handle) override;
	virtual int GetLineHeight(Rml::FontFaceHandle handle) override;
	virtual int GetBaseline(Rml::FontFaceHandle handle) override;
	virtual float GetUnderline(Rml::FontFaceHandle handle, float& thickness) override;
	virtual int GetStringWidth(
		Rml::FontFaceHandle handle,
		const Rml::String& string,
		Rml::Character prior_character = Rml::Character::Null) override;
	virtual int GenerateString(
		Rml::FontFaceHandle face_handle,
		Rml::FontEffectsHandle font_effects_handle,
		const Rml::String& string,
		const Rml::Vector2f& position,
		const Rml::Colourb& colour,
		float opacity,
		Rml::GeometryList& geometry) override;
	virtual int GetVersion(Rml::FontFaceHandle handle) override;
	virtual void ReleaseFontResources() override;

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