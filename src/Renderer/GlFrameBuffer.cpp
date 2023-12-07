#include "GlFrameBuffer.h"
#include "GlCommon.h"

GlFrameBuffer::GlFrameBuffer(const std::string& name) 
	: m_name(name)
{
}

GlFrameBuffer::~GlFrameBuffer()
{
	disposeFrameBuffer();
}

void GlFrameBuffer::setName(const std::string& name)
{
	m_name = name;
}

void GlFrameBuffer::setFramebuffer(GLuint framebuffer)
{
	disposeFrameBuffer();
	m_glFrameBufferId = framebuffer;
	m_needsInit = true;
}

std::string GlFrameBuffer::getName() const
{
	return m_name;
}

GLuint GlFrameBuffer::getFramebuffer() const
{
	return m_glFrameBufferId;
}

GLuint GlFrameBuffer::getTexture(int index) const
{
	if (index < m_glTextureIds.size())
		return m_glTextureIds[index];
	return -1;
}

void GlFrameBuffer::setNumAttachments(int n)
{
	m_numAttachments = n;
	m_needsInit = true;
}

int GlFrameBuffer::getNumAttachments() const
{
	return m_numAttachments;
}

void GlFrameBuffer::setRenderbuffer(bool b)
{
	m_hasRenderBuffer = b;
	m_needsInit = true;
}

bool GlFrameBuffer::hasRenderbuffer() const
{
	return m_hasRenderBuffer;
}

void GlFrameBuffer::setSize(int width, int height)
{
	m_width = width;
	m_height = height;

	m_needsInit = true;
}

void GlFrameBuffer::getSize(int* width, int* height) const
{
	*width = m_width;
	*height = m_height;
}

void GlFrameBuffer::createFrameBuffer()
{
	if (m_glFrameBufferId == 0)
	{
		m_needsInit = false;
		return;
	}

	// Delete old framebuffer
	if (m_glFrameBufferId != -1)
		glDeleteFramebuffers(1, &m_glFrameBufferId);
	glDeleteTextures((GLsizei)m_glTextureIds.size(), m_glTextureIds.data());
	std::vector<uint32_t>().swap(m_glTextureIds);
	if (m_glRenderBufferID != -1)
		glDeleteRenderbuffers(1, &m_glRenderBufferID);

	// Create new framebuffer
	glGenFramebuffers(1, &m_glFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_glFrameBufferId);

	// Create attachments
	for (int i = 0; i < m_numAttachments; i++)
	{
		uint32_t texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture, 0);
		m_glTextureIds.push_back(texture);
	}

	// Create renderbuffer
	if (m_hasRenderBuffer)
	{
		glGenRenderbuffers(1, &m_glRenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_glRenderBufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_glRenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_needsInit = false;
}

bool GlFrameBuffer::needsInit() const
{
	return m_needsInit;
}

void GlFrameBuffer::disposeFrameBuffer()
{
	if (m_glFrameBufferId == 0)
		return;

	if (m_glFrameBufferId != -1)
		glDeleteFramebuffers(1, &m_glFrameBufferId);
	m_glFrameBufferId = -1;
	glDeleteTextures((GLsizei)m_glTextureIds.size(), m_glTextureIds.data());
	std::vector<uint32_t>().swap(m_glTextureIds);
	if (m_glRenderBufferID != -1)
	{
		glDeleteRenderbuffers(1, &m_glRenderBufferID);
		m_glRenderBufferID = -1;
	}
}

bool GlFrameBuffer::bindFrameBuffer()
{
	static const int k_maxAttachments = 16;

	if (m_numAttachments > k_maxAttachments)
		return false;

	if (m_bIsBound)
		return false;

	// Cache the last viewport dimensions
	GLint last_viewport[4];
	glGetIntegerv(GL_VIEWPORT, last_viewport);

	// Change the viewport to match the frame buffer texture
	glViewport(0, 0, m_width, m_height);

	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, m_glRenderBufferID);

	//TODO: Apply scoped rendering flags
	//glStateScope.getStackState().disableFlag(eGlStateFlagType::depthTest);

	GLenum attachments[k_maxAttachments];
	for (int i = 0; i < m_numAttachments; i++)
	{
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(m_numAttachments, attachments);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_bIsBound= true;
	return true;
}

void GlFrameBuffer::unbindFrameBuffer()
{
	if (m_bIsBound)
	{
		// Unbind the layer frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Restore the viewport
		glViewport(m_lastiewport[0], m_lastiewport[1], (GLsizei)m_lastiewport[2], (GLsizei)m_lastiewport[3]);

		m_bIsBound= false;
	}
}