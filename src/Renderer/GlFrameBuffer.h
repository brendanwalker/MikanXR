#pragma once

#include "GlTypesFwd.h"

#include <string>
#include <vector>
#include <stdint.h>

class GlFrameBuffer
{
public:
	GlFrameBuffer()= default;
	GlFrameBuffer(const std::string& name);
	~GlFrameBuffer();

	void createFrameBuffer();
	bool needsInit() const;
	void disposeFrameBuffer();

	bool bindFrameBuffer();
	void unbindFrameBuffer();

	void setName(const std::string& name);
	void setFramebuffer(GLuint framebuffer);
	std::string getName() const;
	GLuint getFramebuffer()  const;
	GLuint getTexture(int index)  const;
	void setNumAttachments(int n);
	int getNumAttachments()  const;
	void setRenderbuffer(bool b);
	bool hasRenderbuffer()  const;
	void setSize(int width, int height);
	void getSize(int* width, int* height)  const;

private:
	std::string m_name;
	GLuint m_glFrameBufferId= -1;
	int m_numAttachments= 0;
	bool m_hasRenderBuffer= false;

	int m_width= 800;
	int m_height= 600;

	bool m_needsInit= false;
	bool m_bIsBound= false;

	std::vector<GLuint> m_glTextureIds;
	GLuint m_glRenderBufferID= -1;

	GLint m_lastiewport[4];
};
