#include "GlFrameBuffer.h"
#include "GlCommon.h"
#include "GLStateStack.h"
#include "GlStateModifiers.h"
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
	disposeResources();

	switch (m_frameBufferType)
	{
		case eFrameBufferType::COLOR:
			m_bIsValid= createColorFrameBuffer();
			break;
		case eFrameBufferType::DEPTH:
			m_bIsValid= createDepthFrameBuffer();
			break;
	}

	return m_bIsValid;
}

bool GlFrameBuffer::createColorFrameBuffer()
{
	// Cache the current frame buffer id
	GLint prevFrameBufferID= 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFrameBufferID);

	// Create new frame buffer
	glGenFramebuffers(1, &m_glFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_glFrameBufferId);
	if (!m_name.empty())
	{
		glObjectLabel(GL_FRAMEBUFFER, m_glFrameBufferId, -1, m_name.c_str());
	}

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
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture->getGlTextureId(), 0);

	// Create depth render buffer attachment for the depth (don't need to read back the depth buffer)
	glGenRenderbuffers(1, &m_glRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_glRenderBufferID);
	if (!m_name.empty())
	{
		glObjectLabel(GL_RENDERBUFFER, m_glRenderBufferID, -1, m_name.c_str());
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_glRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	bool bSuccess = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!bSuccess)
	{
		MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, prevFrameBufferID);

	return bSuccess;
}

bool GlFrameBuffer::createDepthFrameBuffer()
{
	// Cache the current frame buffer id
	GLint prevFrameBufferID = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFrameBufferID);

	// Create new frame buffer
	glGenFramebuffers(1, &m_glFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_glFrameBufferId);
	if (!m_name.empty())
	{
		glObjectLabel(GL_FRAMEBUFFER, m_glFrameBufferId, -1, m_name.c_str());
	}

	// Create a color render buffer attachment (don't need to read back the color buffer)
	glGenRenderbuffers(1, &m_glRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_glRenderBufferID);
	if (!m_name.empty())
	{
		glObjectLabel(GL_RENDERBUFFER, m_glRenderBufferID, -1, m_name.c_str());
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_glRenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Create a depth attachment texture with a double buffered pixel-buffer-object for reading
	if (!m_texture)
	{
		assert(!m_bIsExternalTexture);
		m_texture = std::make_shared<GlTexture>();
		m_texture->setSize(m_width, m_height);
		m_texture->setTextureFormat(GL_DEPTH_COMPONENT32F);
		m_texture->setBufferFormat(GL_DEPTH_COMPONENT);
		m_texture->setGenerateMipMap(false);
		// Assumption: Depth textures are not read back to the CPU, so no PBO is needed
		m_texture->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::NoPBO);
		m_texture->createTexture();
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture->getGlTextureId(), 0);

	m_bIsValid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!m_bIsValid)
	{
		MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, prevFrameBufferID);

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

void GlFrameBuffer::bindObject(GlState& glState)
{
	if (!m_bIsBound && m_glFrameBufferId != -1)
	{
		// Change the viewport to match the frame buffer texture
		glStateSetViewport(glState, 0, 0, m_width, m_height);

		// Cache the last frame buffer binding
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_lastGlFrameBufferId);

		// Bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, m_glFrameBufferId);

		switch (m_frameBufferType)
		{
			case GlFrameBuffer::eFrameBufferType::COLOR:
				{
					glStateSetDrawBuffer(glState, eGlFrameBuffer::COLOR_ATTACHMENT0);
					glStateSetReadBuffer(glState, eGlFrameBuffer::COLOR_ATTACHMENT0);
					glStateSetClearColor(glState, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				}
				break;
			case GlFrameBuffer::eFrameBufferType::DEPTH:
				{
					// Since we only care about depth, tell OpenGL we're not going to render any color data
					glStateSetDrawBuffer(glState, eGlFrameBuffer::NONE);
					glStateSetReadBuffer(glState, eGlFrameBuffer::NONE);
					glClear(GL_DEPTH_BUFFER_BIT);
					glState.enableFlag(eGlStateFlagType::depthTest);
				}
				break;
			default:
				break;
		}

		m_bIsBound = true;
	}
}

void GlFrameBuffer::unbindObject()
{
	if (m_bIsBound)
	{
		// Unbind the layer frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_lastGlFrameBufferId);

		m_bIsBound= false;
	}
}