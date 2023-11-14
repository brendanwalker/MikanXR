#pragma once

#include "GlTypesFwd.h"

#include <string>
#include <vector>
#include <stdint.h>

class GlFrameBuffer
{
public:
	GlFrameBuffer();
	GlFrameBuffer(const std::string& name);
	~GlFrameBuffer();

	void createFrameBuffer();
	bool needsInit() const;
	void disposeFrameBuffer();

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
	GLuint m_glFrameBufferId;
	int m_numAttachments;
	bool m_hasRenderBuffer;

	int m_width;
	int m_height;

	bool m_needsInit;

	std::vector<GLuint> m_glTextureIds;
	GLuint m_glRenderBufferID;
};
