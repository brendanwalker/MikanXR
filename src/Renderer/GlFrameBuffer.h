#pragma once

#include "GlTypesFwd.h"
#include "RendererFwd.h"

#include <string>
#include <vector>
#include <stdint.h>

class GlFrameBuffer
{
public:
	enum class eAttachmentType
	{
		COLOR,
		DEPTH
	};

	GlFrameBuffer()= default;
	GlFrameBuffer(const std::string& name);
	~GlFrameBuffer();

	bool isValid() const { return m_bIsValid; }

	bool createResources();
	void disposeResources();

	bool bindFrameBuffer(class GlState& glState);
	void unbindFrameBuffer();

	void setName(const std::string& name) { m_name = name; }
	void setAttachmentType(eAttachmentType attachmentType) { m_attachmentType = attachmentType; }
	void setSize(int width, int height);
	void setExternalTexture(GlTexturePtr texture);

	std::string getName() const { return m_name; }
	GLuint getGlFrameBufferId() const { return m_glFrameBufferId; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	GlTexturePtr getTexture() const;

private:
	bool createColorAttachment();
	bool createDepthAttachment();

private:
	std::string m_name;
	eAttachmentType m_attachmentType= eAttachmentType::COLOR;

	int m_width= 800;
	int m_height= 600;

	GlTexturePtr m_texture;
	bool m_bIsExternalTexture= false;
	GLuint m_glRenderBufferID= -1;

	// Cached GLState
	GLuint m_glFrameBufferId = -1;
	GLint m_lastGlFrameBufferId = 0;
	GLint m_lastiewport[4];

	bool m_bIsBound= false;
	bool m_bIsValid= false;
};
