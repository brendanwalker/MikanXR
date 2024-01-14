#include "GlFrameBuffer.h"
#include "GlCommon.h"
#include "GLStateStack.h"
#include "GlTexture.h"
#include "Logger.h"

#include <assert.h>

GlFrameBuffer::GlFrameBuffer(const std::string& name) 
	: m_name(name)
{
}

GlFrameBuffer::~GlFrameBuffer()
{
	disposeResources();
}

bool GlFrameBuffer::createResources()
{
	if (!m_bIsValid)
	{
		disposeResources();

		// Create new frame buffer
		glGenFramebuffers(1, &m_glFrameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_glFrameBufferId);

		// Create a color attachment texture with a double buffered pixel-buffer-object for reading
		if (!m_texture)
		{
			assert(!m_bIsExternalTexture);
			m_texture = std::make_shared<GlTexture>();
			m_texture->setSize(m_width, m_height);
			m_texture->setTextureFormat(GL_RGB);
			m_texture->setBufferFormat(GL_RGB);
			m_texture->setGenerateMipMap(false);
			m_texture->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBORead);
			m_texture->createTexture();

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->getGlTextureId(), 0);
		}

		// Create render buffer
		glGenRenderbuffers(1, &m_glRenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_glRenderBufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_glRenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		m_bIsValid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		if (!m_bIsValid)
		{
			MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return m_bIsValid;
}

void GlFrameBuffer::setSize(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;
		m_bIsValid = false;
	}
}

void GlFrameBuffer::setExternalTexture(GlTexturePtr texture)
{
	if (m_texture != texture)
	{
		if (texture)
		{
			m_texture = texture;
			m_bIsExternalTexture = true;
		}
		else
		{
			m_texture = nullptr;
			m_bIsExternalTexture = false;
		}
		m_bIsValid = false;
	}
}

GlTexturePtr GlFrameBuffer::getTexture() const
{
	return m_texture;
}

void GlFrameBuffer::disposeResources()
{
	if (m_glRenderBufferID != -1)
	{
		glDeleteRenderbuffers(1, &m_glRenderBufferID);
		m_glRenderBufferID = -1;
	}

	if (!m_bIsExternalTexture)
	{
		m_texture = nullptr;
	}

	if (m_glFrameBufferId != -1)
	{
		glDeleteFramebuffers(1, &m_glFrameBufferId);
		m_glFrameBufferId = -1;
	}

	m_bIsValid= false;
}

bool GlFrameBuffer::bindFrameBuffer()
{
	if (m_glRenderBufferID != -1 && !m_bIsBound)
	{
		// Cache the last viewport dimensions
		GLint last_viewport[4];
		glGetIntegerv(GL_VIEWPORT, last_viewport);

		// Change the viewport to match the frame buffer texture
		glViewport(0, 0, m_width, m_height);

		// Bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, m_glRenderBufferID);

		GLenum attachments[1]= {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, attachments);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_bIsBound = true;

		return true;
	}

	return false;
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