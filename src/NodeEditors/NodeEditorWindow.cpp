//-- includes -----
#include "NodeEditorWindow.h"
#include "Logger.h"

#include "GlCommon.h"
#include "GlStateStack.h"
#include "GlShaderCache.h"
#include "MathGLM.h"
#include "SdlWindow.h"

#include <easy/profiler.h>

//-- public methods -----
NodeEditorWindow::NodeEditorWindow()
	: m_sdlWindow(SdlWindowUniquePtr(new SdlWindow))
	, m_glStateStack(GlStateStackUniquePtr(new GlStateStack))
	, m_shaderCache(GlShaderCacheUniquePtr(new GlShaderCache))
{}

NodeEditorWindow::~NodeEditorWindow()
{
}

GlLineRenderer* NodeEditorWindow::getLineRenderer()
{
	return nullptr;
}

GlTextRenderer* NodeEditorWindow::getTextRenderer()
{
	return nullptr;
}

GlStateStack& NodeEditorWindow::getGlStateStack()
{
	return *m_glStateStack.get();
}

bool NodeEditorWindow::startup()
{
	EASY_FUNCTION();

	bool success = true;

	MIKAN_LOG_INFO("NodeEditorWindow::init()") << "Initializing NodeEditorWindow";

	static const int k_node_window_pixel_width = 1080;
	static const int k_node_window_pixel_height = 720;

	auto windowTitle = "Node Editor";
	m_sdlWindow
		->setTitle(windowTitle)
		->setSize(k_node_window_pixel_width, k_node_window_pixel_height);
	if (!m_sdlWindow->startup())
	{
		MIKAN_LOG_ERROR("NodeEditorWindow::startup") << "Unable to initialize main SDK window: ";
		success = false;
	}

	if (success && !m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("NodeEditorWindow::startup") << "Failed to initialize shader cache!";
		success = false;
	}

	if (success)
	{
		static const glm::vec4 k_clear_color = glm::vec4(0.45f, 0.45f, 0.5f, 1.f);

		glClearColor(k_clear_color.r, k_clear_color.g, k_clear_color.b, k_clear_color.a);
		glViewport(0, 0, m_sdlWindow->getWidth(), m_sdlWindow->getHeight());

		// Set default state flags at the base of the stack
		m_glStateStack->pushState()
			.enableFlag(eGlStateFlagType::light0)
			.enableFlag(eGlStateFlagType::texture2d)
			.enableFlag(eGlStateFlagType::depthTest)
			.disableFlag(eGlStateFlagType::cullFace)
			// This has to be enabled since the point drawing shader will use gl_PointSize.
			.enableFlag(eGlStateFlagType::programPointSize);
	}

	return success;
}

void NodeEditorWindow::render()
{
	m_sdlWindow->renderBegin();
	m_isRenderingUI= true;
	m_isRenderingUI= false;
	m_sdlWindow->renderEnd();
}

void NodeEditorWindow::shutdown()
{
	m_glStateStack = nullptr;

	if (m_shaderCache != nullptr)
	{
		m_shaderCache->shutdown();
		m_shaderCache = nullptr;
	}

	if (m_sdlWindow != nullptr)
	{
		m_sdlWindow->shutdown();
		m_sdlWindow = nullptr;
	}
}

float NodeEditorWindow::getWidth() const
{
	return (float)m_sdlWindow->getWidth();
}

float NodeEditorWindow::getHeight() const
{
	return (float)m_sdlWindow->getHeight();
}

float NodeEditorWindow::getAspectRatio() const
{
	return (float)m_sdlWindow->getAspectRatio();
}

bool NodeEditorWindow::onSDLEvent(const SDL_Event* event)
{
	m_sdlWindow->onSDLEvent(event);

	return true;
}