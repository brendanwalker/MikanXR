#include "SharedTextureWriterBackend.h"
#include "SharedTextureLogger.h"
#include "SpoutGLDepthTexturePacker.h"
#include "SpoutLibrary.h"

// SPOUTLIBRARY::SetSenderFormat takes a DXGI format even on the OpenGL path
#include <dxgiformat.h>

#include <sstream>
#include <string>

namespace
{
// Spout keeps its log state in module globals, so each of the two Spout code paths
// only has to be configured once per process.
void applySpoutLibraryLogPolicy(SPOUTLIBRARY* spoutLibrary)
{
	static bool s_bPolicyApplied= false;
	if (spoutLibrary == nullptr || s_bPolicyApplied)
		return;
	s_bPolicyApplied= true;

	switch (getSpoutLogTarget())
	{
	case SpoutLogTarget::console:
		spoutLibrary->EnableSpoutLog();
		break;
	case SpoutLogTarget::file:
		spoutLibrary->EnableSpoutLogFile(k_spoutSenderLogFileName);
		break;
	case SpoutLogTarget::none:
		return;
	}

	spoutLibrary->SetSpoutLogLevel(LibLogLevel::SPOUT_LOG_VERBOSE);
}
} // namespace

// -- SpoutOpenGLTextureWriter -----
class SpoutOpenGLTextureWriter : public ISharedTextureWriterBackend
{
public:
	SpoutOpenGLTextureWriter(const SharedTextureWriterContext& context)
		: m_context(context)
		, m_logger(*context.logger)
		, m_spoutColorFrame(nullptr)
		, m_spoutDepthFrame(nullptr)
		, m_spoutShadowFrame(nullptr)
	{
	}

	virtual ~SpoutOpenGLTextureWriter() { dispose(); }

	bool init() override
	{
		const SharedTextureDescriptor* descriptor= m_context.descriptor;
		bool bSuccess= true;

		dispose();

		m_spoutColorFrame= GetSpout();
		if (m_spoutColorFrame == nullptr)
		{
			m_logger.log(SharedTextureLogLevel::error, "SpoutTextureWriter - Failed to open spout api");
			return false;
		}

		applySpoutLibraryLogPolicy(m_spoutColorFrame);

		if (descriptor->color_buffer_type == SharedColorBufferType::RGBA32
			|| descriptor->color_buffer_type == SharedColorBufferType::BGRA32
			|| descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
		{
			m_spoutColorFrame->SetSenderName(m_context.colorSenderName.c_str());

			if (descriptor->color_buffer_type == SharedColorBufferType::RGBA16F)
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R16G16B16A16_FLOAT);
			else if (descriptor->color_buffer_type == SharedColorBufferType::BGRA32)
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutColorFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutColorFrame->SetFrameCount(m_context.bEnableFrameCounter);
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - color buffer type not supported: ";
			ss << (int)descriptor->color_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		if (descriptor->depth_buffer_type == SharedDepthBufferType::PACK_DEPTH_RGBA
			|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
			|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
		{
			m_spoutDepthFrame= GetSpout();
			if (m_spoutDepthFrame == nullptr)
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutOpenGLTextureWriter::init() - Failed to open spout api for depth");
				return false;
			}

			m_spoutDepthFrame->SetSenderName(m_context.depthSenderName.c_str());

			// Initialize the depth texture packer if we are sending float depth textures
			if (descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_DEVICE_DEPTH
				|| descriptor->depth_buffer_type == SharedDepthBufferType::FLOAT_SCENE_DEPTH)
			{
				m_depthTexturePacker= new SpoutGLDepthTexturePacker(m_logger, m_spoutDepthFrame, descriptor);
				if (!m_depthTexturePacker->init())
				{
					m_logger.log(SharedTextureLogLevel::info,
								 "SpoutOpenGLTextureWriter::init() - Error initializing float depth packer");
					return false;
				}
			}

			m_spoutDepthFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);
			m_spoutDepthFrame->SetFrameCount(m_context.bEnableFrameCounter);
		}
		else if (descriptor->depth_buffer_type == SharedDepthBufferType::NODEPTH)
		{
			m_spoutDepthFrame= nullptr;
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - depth buffer type not supported: ";
			ss << (int)descriptor->depth_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		// Initialize the (optional) shadow spout frame. It's a color-like RGBA8/BGRA8 buffer,
		// so no depth packer is needed - it mirrors the color frame path.
		if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA32
			|| descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32
			|| descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
		{
			m_spoutShadowFrame= GetSpout();
			if (m_spoutShadowFrame == nullptr)
			{
				m_logger.log(SharedTextureLogLevel::error,
							 "SpoutOpenGLTextureWriter::init() - Failed to open spout api for shadow");
				return false;
			}

			m_spoutShadowFrame->SetSenderName(m_context.shadowSenderName.c_str());

			if (descriptor->shadow_buffer_type == SharedShadowBufferType::RGBA16F)
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R16G16B16A16_FLOAT);
			else if (descriptor->shadow_buffer_type == SharedShadowBufferType::BGRA32)
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_B8G8R8A8_UNORM);
			else
				m_spoutShadowFrame->SetSenderFormat((DWORD)DXGI_FORMAT_R8G8B8A8_UNORM);

			m_spoutShadowFrame->SetFrameCount(m_context.bEnableFrameCounter);
		}
		else if (descriptor->shadow_buffer_type == SharedShadowBufferType::NOSHADOW)
		{
			m_spoutShadowFrame= nullptr;
		}
		else
		{
			std::stringstream ss;
			ss << "SpoutOpenGLTextureWriter::init() - shadow buffer type not supported: ";
			ss << (int)descriptor->shadow_buffer_type;
			m_logger.log(SharedTextureLogLevel::info, ss.str());
			bSuccess= false;
		}

		return bSuccess;
	}

	void dispose()
	{
		if (m_depthTexturePacker != nullptr)
		{
			delete m_depthTexturePacker;
			m_depthTexturePacker= nullptr;
		}

		if (m_spoutColorFrame != nullptr)
		{
			m_spoutColorFrame->Release();
			m_spoutColorFrame= nullptr;
		}

		if (m_spoutDepthFrame != nullptr)
		{
			m_spoutDepthFrame->Release();
			m_spoutDepthFrame= nullptr;
		}

		if (m_spoutShadowFrame != nullptr)
		{
			m_spoutShadowFrame->Release();
			m_spoutShadowFrame= nullptr;
		}
	}

	bool writeColorFrameTexture(void* apiTexturePtr) override
	{
		const GLuint textureID= *(GLuint*)apiTexturePtr;

		if (m_spoutColorFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_context.descriptor;

			return m_spoutColorFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
		}

		return false;
	}

	bool writeDepthFrameTexture(void* apiTexturePtr, float zNear, float zFar) override
	{
		const GLuint textureID= *(GLuint*)apiTexturePtr;

		if (m_spoutDepthFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_context.descriptor;

			if (m_depthTexturePacker != nullptr)
			{
				// Convert the float depth texture to a RGBA8 texture using a shader
				// (Spout can only send RGBA8 textures)
				GLuint packedDepthTexture= m_depthTexturePacker->packDepthTexture(textureID, zNear, zFar);

				if (packedDepthTexture != 0)
				{
					return m_spoutDepthFrame->SendTexture(packedDepthTexture, GL_TEXTURE_2D, descriptor->width,
														  descriptor->height);
				}
			}
			else
			{
				return m_spoutDepthFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
			}
		}

		return false;
	}

	bool writeShadowFrameTexture(void* apiTexturePtr) override
	{
		const GLuint textureID= *(GLuint*)apiTexturePtr;

		if (m_spoutShadowFrame != nullptr)
		{
			const SharedTextureDescriptor* descriptor= m_context.descriptor;

			return m_spoutShadowFrame->SendTexture(textureID, GL_TEXTURE_2D, descriptor->width, descriptor->height);
		}

		return false;
	}

	void* getPackDepthTextureResourcePtr() const override
	{
		return m_depthTexturePacker != nullptr ? (void*)m_depthTexturePacker->getPackedDepthTextureResourcePtr()
											   : nullptr;
	}

private:
	const SharedTextureWriterContext& m_context;
	SharedTextureLogger& m_logger;
	SPOUTLIBRARY* m_spoutColorFrame;
	SPOUTLIBRARY* m_spoutDepthFrame;
	SPOUTLIBRARY* m_spoutShadowFrame;
	SpoutGLDepthTexturePacker* m_depthTexturePacker= nullptr;
};

ISharedTextureWriterBackendPtr createOpenGLTextureWriter(const SharedTextureWriterContext& context)
{
	return std::make_unique<SpoutOpenGLTextureWriter>(context);
}
