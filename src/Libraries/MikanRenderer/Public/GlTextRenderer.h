#pragma once

#include "RendererFwd.h"
#include "TextStyle.h"

#include <vector>

#include "glm/ext/vector_float2.hpp"

//-- macros -----
#if defined(__clang__) || defined(__GNUC__)
#define GLYPH_PRINTFARGS(FMT) __attribute__((format(printf, FMT, (FMT+1))))
#else
#define GLYPH_PRINTFARGS(FMT)
#endif

class GlTextRenderer
{
public:
	GlTextRenderer(class IMkWindow* ownerWindow);
	virtual ~GlTextRenderer();

	bool startup();
	void render();
	void shutdown();

	void addTextAtScreenPosition(const TextStyle& style, const glm::vec2& screenCoords, const std::wstring& text);

protected:
	struct BakedTextQuad
	{
		GlTexturePtr texture;
		int startVertexIndex;
	};

	struct TextQuadVertex
	{
		glm::vec2 position;
		glm::vec2 texCoords;
	};

	int allocateTextQuadVertices(int vertexCount);
	void setTextQuadVertex(int index, const glm::vec2& position, const glm::vec2& texCoords);

private:
	static const int kMaxTextQuads= 1024;
	class IMkWindow* m_ownerWindow= nullptr;

	std::vector<BakedTextQuad> m_bakedTextQuads;
	unsigned int m_textQuadVAO= 0;
	unsigned int m_textQuadVBO= 0;
	int m_textQuadVertexCount= 0;
	int m_maxTextQuadVertexCount;
	TextQuadVertex* m_textQuadVertices;
	GlMaterialConstPtr m_textMaterial;
	GlMaterialInstancePtr m_textMaterialInstance;
};

//-- drawing methods -----
void drawTextAtWorldPosition(
	const TextStyle& style, 
	const glm::vec3& position, 
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);
void drawTextAtScreenPosition(
	const TextStyle& style, 
	const glm::vec2& screenCoords, 
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);
void drawTextAtCameraPosition(
	const TextStyle& style,
	const float cameraWidth, const float cameraHeight,
	const glm::vec2& cameraCoords,
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);
void drawTextAtTrackerPosition(
	const TextStyle& style,
	const float trackerWidth, const float trackerHeight,
	const glm::vec2& trackerCoords,
	const wchar_t* format, ...) GLYPH_PRINTFARGS(2);