#include "GlCommon.h"
#include "GlGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "IMkTextRenderer.h"
#include "IMkShaderCache.h"
#include "IMkTextureCache.h"
#include "MkStateStack.h"
#include "Logger.h"

#include <assert.h>

// -- GL Debug Callback --
static void APIENTRY GLDebugMessageCallback(
	GLenum glMesgSource,
	GLenum glMesgType,
	GLuint glMesgId,
	GLenum glMesgSeverity,
	GLsizei length,
	const GLchar* szGlMessage,
	const void* userParam);

// -- GlGraphicsContext --
GlGraphicsContext::GlGraphicsContext(class IMkFontManager* fontManager)
	: m_fontManager(fontManager)
{
}

GlGraphicsContext::~GlGraphicsContext()
{
	if (m_stateStack || m_lineRenderer || m_textRenderer || m_shaderCache || m_textureCache)
	{
		shutdown();
	}
}

void GlGraphicsContext::onNativeContextCreated(void* nativeContext)
{
	m_glContext= nativeContext;
}

void GlGraphicsContext::onWindowSizeChanged(int width, int height)
{
	m_width= (float)width;
	m_height= (float)height;
}

bool GlGraphicsContext::startup()
{
	assert(m_glContext != nullptr);

	// Initialize GL Extension Wrangler (GLEW)
	GLenum err;
	glewExperimental= GL_TRUE; // Please expose OpenGL 3.x+ interfaces
	err= glewInit();
	if (err == GLEW_OK)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugMessageCallback, m_glContext);
	}
	else
	{
		MIKAN_LOG_ERROR("GlGraphicsContext::startup") << "Unable to initialize glew openGL backend: " << glewGetErrorString(err);
		return false;
	}

	// Create state stack
	m_stateStack= MkStateStackUniquePtr(new MkStateStack(this));

	// Create shader cache and register built-in shaders
	m_shaderCache= createMkShaderCache(this);
	if (!m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("GlGraphicsContext::startup") << "Failed to startup shader cache";
		return false;
	}

	// Create texture cache
	m_textureCache= createMkTextureCache(this);
	if (!m_textureCache->startup())
	{
		MIKAN_LOG_ERROR("GlGraphicsContext::startup") << "Failed to startup texture cache";
		return false;
	}

	// Create line renderer
	m_lineRenderer= createMkLineRenderer(this);
	if (!m_lineRenderer->startup())
	{
		MIKAN_LOG_ERROR("GlGraphicsContext::startup") << "Failed to startup line renderer";
		return false;
	}

	// Create text renderer (only if font manager is provided)
	if (m_fontManager != nullptr)
	{
		m_textRenderer= createMkTextRenderer(this, m_fontManager);
		if (!m_textRenderer->startup())
		{
			MIKAN_LOG_ERROR("GlGraphicsContext::startup") << "Failed to startup text renderer";
			return false;
		}
	}

	return true;
}

bool GlGraphicsContext::renderBegin()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool GlGraphicsContext::renderEnd()
{
	return true;
}

void GlGraphicsContext::shutdown()
{
	if (m_textRenderer)
	{
		m_textRenderer->shutdown();
		m_textRenderer= nullptr;
	}

	if (m_lineRenderer)
	{
		m_lineRenderer->shutdown();
		m_lineRenderer= nullptr;
	}

	if (m_textureCache)
	{
		m_textureCache->shutdown();
		m_textureCache= nullptr;
	}

	if (m_shaderCache)
	{
		m_shaderCache->shutdown();
		m_shaderCache= nullptr;
	}

	m_stateStack= nullptr;
	m_glContext= nullptr;
}

MkStateStack& GlGraphicsContext::getMkStateStack()
{
	assert(m_stateStack != nullptr);
	return *m_stateStack;
}

IMkLineRenderer* GlGraphicsContext::getLineRenderer()
{
	return m_lineRenderer.get();
}

IMkTextRenderer* GlGraphicsContext::getTextRenderer()
{
	return m_textRenderer.get();
}

IMkShaderCache* GlGraphicsContext::getShaderCache()
{
	return m_shaderCache.get();
}

IMkTextureCache* GlGraphicsContext::getTextureCache()
{
	return m_textureCache.get();
}

// -- Factory function --
IMkGraphicsContextPtr createMkGraphicsContext(eGraphicsAPI api, class IMkFontManager* fontManager)
{
	// Currently only OpenGL is supported
	assert(api == eGraphicsAPI::OpenGL);
	return std::make_shared<GlGraphicsContext>(fontManager);
}

// -- GL Debug Callback Implementation --
enum class eGLErrorSeverity : int
{
	unknown,
	notification,
	low,
	medium,
	high,
};
static const char* g_szGLErrorSeverityNames[]= {
	"UNKNOWN",
	"NOTIFICATION",
	"LOW",
	"MEDIUM",
	"HIGH",
};

static void APIENTRY GLDebugMessageCallback(
	GLenum glMesgSource,
	GLenum glMesgType,
	GLuint glMesgId,
	GLenum glMesgSeverity,
	GLsizei length,
	const GLchar* szGlMessage,
	const void* userParam)
{
	std::string glErrorSourceStr;
	std::string glErrorTypeStr;

	switch (glMesgSource)
	{
	case GL_DEBUG_SOURCE_API:
		glErrorSourceStr= "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		glErrorSourceStr= "WINDOW SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		glErrorSourceStr= "SHADER COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		glErrorSourceStr= "THIRD PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		glErrorSourceStr= "APPLICATION";
		break;
	default:
		glErrorSourceStr= "UNKNOWN";
		break;
	}

	eGLErrorSeverity minSeverity= eGLErrorSeverity::low;
	switch (glMesgType)
	{
	case GL_DEBUG_TYPE_ERROR:
		glErrorTypeStr= "ERROR";
		minSeverity= eGLErrorSeverity::notification;
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		glErrorTypeStr= "DEPRECATED BEHAVIOR";
		minSeverity= eGLErrorSeverity::high;
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		glErrorTypeStr= "UDEFINED BEHAVIOR";
		minSeverity= eGLErrorSeverity::notification;
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		glErrorTypeStr= "PORTABILITY";
		minSeverity= eGLErrorSeverity::high;
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		glErrorTypeStr= "PERFORMANCE";
		minSeverity= eGLErrorSeverity::high;
		break;
	case GL_DEBUG_TYPE_OTHER:
		glErrorTypeStr= "OTHER";
		minSeverity= eGLErrorSeverity::low;
		break;
	case GL_DEBUG_TYPE_MARKER:
		glErrorTypeStr= "MARKER";
		minSeverity= eGLErrorSeverity::low;
		break;
	default:
		glErrorTypeStr= "UNKNOWN";
		minSeverity= eGLErrorSeverity::low;
		break;
	}

	eGLErrorSeverity eventSeverity= eGLErrorSeverity::unknown;
	switch (glMesgSeverity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		eventSeverity= eGLErrorSeverity::high;
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		eventSeverity= eGLErrorSeverity::medium;
		break;
	case GL_DEBUG_SEVERITY_LOW:
		eventSeverity= eGLErrorSeverity::low;
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		eventSeverity= eGLErrorSeverity::notification;
		break;
	default:
		eventSeverity= eGLErrorSeverity::unknown;
		break;
	}

	if ((int)eventSeverity >= (int)minSeverity)
	{
		switch (eventSeverity)
		{
		case eGLErrorSeverity::high:
		case eGLErrorSeverity::unknown:
			MIKAN_LOG_ERROR("GlDebugCallback") << "OpenGL debug event [" << glMesgId << "]: "
											   << glErrorTypeStr << " of "
											   << g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
											   << glErrorSourceStr << ": "
											   << szGlMessage;
			break;
		case eGLErrorSeverity::medium:
			MIKAN_LOG_WARNING("GlDebugCallback") << "OpenGL debug event [" << glMesgId << "]: "
												 << glErrorTypeStr << " of "
												 << g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
												 << glErrorSourceStr << ": "
												 << szGlMessage;
			break;
		case eGLErrorSeverity::low:
		case eGLErrorSeverity::notification:
			MIKAN_LOG_INFO("GlDebugCallback") << "OpenGL debug event [" << glMesgId << "]: "
											  << glErrorTypeStr << " of "
											  << g_szGLErrorSeverityNames[(int)eventSeverity] << " severity, raised from "
											  << glErrorSourceStr << ": "
											  << szGlMessage;
			break;
		default:
			assert(false);
		}

#ifdef _DEBUG
		if ((int)eventSeverity >= (int)eGLErrorSeverity::high)
		{
			__debugbreak();
		}
#endif
	}
}
