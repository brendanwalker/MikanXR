#include "IMkFrameBuffer.h"
#include "GlCommon.h"
#include "GLStateStack.h"
#include "GlStateModifiers.h"
#include "IMkTexture.h"
#include "Logger.h"

#include <assert.h>

namespace GlFrameBufferUtils
{
	GLenum getGlColorFormat(IMkFrameBuffer::eColorFormat colorFormat)
	{
		switch (colorFormat)
		{
			case IMkFrameBuffer::eColorFormat::RGB:
				return GL_RGB;
			case IMkFrameBuffer::eColorFormat::RGBA:
				return GL_RGBA;
			default:
				return GL_RGB;
		}
	}
}

class GlFrameBuffer: public IMkFrameBuffer
{
public:
	enum class eFrameBufferType
	{
		COLOR,
		DEPTH,
		COLOR_AND_DEPTH,
	};

	enum class eColorFormat
	{
		RGB,
		RGBA,
	};

	GlFrameBuffer() = default;
	GlFrameBuffer(const std::string& name) : m_name(name)
	{
		m_clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	virtual ~GlFrameBuffer()
	{
		disposeResources();
	}

	virtual bool isValid() const override 
	{ 
		return m_bIsValid; 
	}

	virtual bool createResources() override
	{
		disposeResources();

		switch (m_frameBufferType)
		{
			case IMkFrameBuffer::eFrameBufferType::COLOR:
				m_bIsValid = createColorFrameBuffer();
				break;
			case IMkFrameBuffer::eFrameBufferType::DEPTH:
				m_bIsValid = createDepthFrameBuffer();
				break;
			case IMkFrameBuffer::eFrameBufferType::COLOR_AND_DEPTH:
				m_bIsValid = createColorAndDepthFrameBuffer();
				break;
		}

		return m_bIsValid;
	}

	bool createColorFrameBuffer()
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

		// Create a color attachment texture with a double buffered pixel-buffer-object for reading
		if (!m_colorTexture)
		{
			const GLenum glColorFormat = GlFrameBufferUtils::getGlColorFormat(m_colorFormat);

			assert(!m_bIsExternalTexture);
			m_colorTexture = CreateMkTexture();
			m_colorTexture->setSize(m_width, m_height);
			m_colorTexture->setTextureFormat(glColorFormat);
			m_colorTexture->setBufferFormat(glColorFormat);
			m_colorTexture->setGenerateMipMap(false);
			m_colorTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBORead);
			m_colorTexture->createTexture();
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getGlTextureId(), 0);

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

	bool createDepthFrameBuffer()
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
		if (!m_depthTexture)
		{
			assert(!m_bIsExternalTexture);
			m_depthTexture = CreateMkTexture();
			m_depthTexture->setSize(m_width, m_height);
			m_depthTexture->setTextureFormat(GL_DEPTH_COMPONENT32F);
			m_depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
			m_depthTexture->setGenerateMipMap(false);
			// Assumption: Depth textures are not read back to the CPU, so no PBO is needed
			m_depthTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::NoPBO);
			m_depthTexture->createTexture();
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture->getGlTextureId(), 0);

		m_bIsValid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		if (!m_bIsValid)
		{
			MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, prevFrameBufferID);

		return m_bIsValid;
	}

	bool createColorAndDepthFrameBuffer()
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

		// Create a color attachment texture with a double buffered pixel-buffer-object for reading
		if (!m_colorTexture)
		{
			const GLenum glColorFormat = GlFrameBufferUtils::getGlColorFormat(m_colorFormat);

			assert(!m_bIsExternalTexture);
			m_colorTexture = CreateMkTexture();
			m_colorTexture->setSize(m_width, m_height);
			m_colorTexture->setTextureFormat(glColorFormat);
			m_colorTexture->setBufferFormat(glColorFormat);
			m_colorTexture->setGenerateMipMap(false);
			m_colorTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::DoublePBORead);
			m_colorTexture->createTexture();
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture->getGlTextureId(), 0);

		// Create a depth attachment texture with a double buffered pixel-buffer-object for reading
		if (!m_depthTexture)
		{
			m_depthTexture = CreateMkTexture();
			m_depthTexture->setSize(m_width, m_height);
			m_depthTexture->setTextureFormat(GL_DEPTH_COMPONENT32F);
			m_depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
			m_depthTexture->setGenerateMipMap(false);
			// Assumption: Depth textures are not read back to the CPU, so no PBO is needed
			m_depthTexture->setPixelBufferObjectMode(IMkTexture::PixelBufferObjectMode::NoPBO);
			m_depthTexture->createTexture();
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture->getGlTextureId(), 0);

		bool bSuccess = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		if (!bSuccess)
		{
			MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, prevFrameBufferID);

		return bSuccess;
	}

	virtual void disposeResources() override
	{
		if (m_glRenderBufferID != -1)
		{
			glDeleteRenderbuffers(1, &m_glRenderBufferID);
			m_glRenderBufferID = -1;
		}

		if (!m_bIsExternalTexture)
		{
			m_colorTexture = nullptr;
		}

		m_depthTexture = nullptr;

		if (m_glFrameBufferId != -1)
		{
			glDeleteFramebuffers(1, &m_glFrameBufferId);
			m_glFrameBufferId = -1;
		}

		m_bIsValid = false;
	}

	virtual void setName(const std::string& name) override
	{ 
		m_name = name; 
	}

	virtual void setFrameBufferType(IMkFrameBuffer::eFrameBufferType frameBufferType) override
	{
		if (m_frameBufferType != frameBufferType)
		{
			m_frameBufferType = frameBufferType;
			m_bIsValid = false;
		}
	}
	
	virtual void setSize(int width, int height) override
	{
		if (m_width != width || m_height != height)
		{
			m_width = width;
			m_height = height;
			m_bIsValid = false;
		}
	}
	
	virtual void setColorFormat(IMkFrameBuffer::eColorFormat colorFormat) override
	{
		if (m_colorFormat != colorFormat)
		{
			m_colorFormat = colorFormat;
			m_bIsValid = false;
		}
	}
	
	void setClearColor(const glm::vec4& clearColor) 
	{ 
		m_clearColor = clearColor; 
	}

	void IMkFrameBuffer::setExternalColorTexture(IMkTexturePtr texture)
	{
		if (m_colorTexture != texture)
		{
			if (texture)
			{
				m_colorTexture = texture;
				m_bIsExternalTexture = true;
			}
			else
			{
				m_colorTexture = nullptr;
				m_bIsExternalTexture = false;
			}
			m_bIsValid = false;
		}
	}

	virtual std::string getName() const override 
	{
		return m_name;
	}

	virtual uint32_t getMkFrameBufferId() const override
	{ 
		return m_glFrameBufferId;
	}

	virtual int getWidth() const override
	{ 
		return m_width;
	}

	virtual int getHeight() const override
	{
		return m_height;
	}

	virtual IMkFrameBuffer::eColorFormat getColorFormat() const override 
	{ 
		return m_colorFormat; 
	}

	virtual IMkTexturePtr getColorTexture() const override
	{
		return m_colorTexture;
	}

	virtual IMkTexturePtr getDepthTexture() const override
	{
		return m_depthTexture;
	}

	virtual GlState* getGlState() const override
	{ 
		return m_glState; 
	}

	virtual void bindObject(GlState& glState) override
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
				case IMkFrameBuffer::eFrameBufferType::COLOR:
					{
						glStateSetDrawBuffer(glState, eIMkFrameBuffer::COLOR_ATTACHMENT0);
						glStateSetReadBuffer(glState, eIMkFrameBuffer::COLOR_ATTACHMENT0);
						glStateSetClearColor(glState, m_clearColor);
						glClear(GL_COLOR_BUFFER_BIT);
					}
					break;
				case IMkFrameBuffer::eFrameBufferType::DEPTH:
					{
						// Since we only care about depth, tell OpenGL we're not going to render any color data
						glStateSetDrawBuffer(glState, eIMkFrameBuffer::NONE);
						glStateSetReadBuffer(glState, eIMkFrameBuffer::NONE);
						glClear(GL_DEPTH_BUFFER_BIT);
						glState.enableFlag(eGlStateFlagType::depthTest);
					}
					break;
				case IMkFrameBuffer::eFrameBufferType::COLOR_AND_DEPTH:
					{
						glStateSetDrawBuffer(glState, eIMkFrameBuffer::COLOR_ATTACHMENT0);
						glStateSetReadBuffer(glState, eIMkFrameBuffer::COLOR_ATTACHMENT0);
						glStateSetClearColor(glState, m_clearColor);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						glState.enableFlag(eGlStateFlagType::depthTest);
					}
					break;
				default:
					break;
			}

			m_bIsBound = true;
		}
	}

	virtual void unbindObject() override
	{
		if (m_bIsBound)
		{
			// Unbind the layer frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, m_lastGlFrameBufferId);

			m_bIsBound = false;
		}
	}

	virtual bool getIsBound() const override 
	{ 
		return m_bIsBound; 
	}

private:
	GlState* m_glState = nullptr;

	std::string m_name;
	IMkFrameBuffer::eFrameBufferType m_frameBufferType = IMkFrameBuffer::eFrameBufferType::COLOR;
	IMkFrameBuffer::eColorFormat m_colorFormat = IMkFrameBuffer::eColorFormat::RGB;
	glm::vec4 m_clearColor;

	int m_width = 800;
	int m_height = 600;

	IMkTexturePtr m_colorTexture;
	bool m_bIsExternalTexture = false;
	IMkTexturePtr m_depthTexture;
	GLuint m_glRenderBufferID = -1;

	// Cached GLState
	GLuint m_glFrameBufferId = -1;
	GLint m_lastGlFrameBufferId = 0;

	bool m_bIsBound = false;
	bool m_bIsValid = false;
};

IMkFrameBufferPtr createMkFrameBuffer()
{
	return std::make_shared<GlFrameBuffer>();
}

IMkFrameBufferPtr createMkFrameBuffer(const std::string& name)
{
	return std::make_shared<GlFrameBuffer>(name);
}