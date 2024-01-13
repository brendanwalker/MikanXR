//-- includes -----
#include "MainWindow.h"
#include "Logger.h"
#include "Version.h"

#include "GlRmlUiRenderer.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include "App.h"
#include "AppStage.h"
#include "FontManager.h"
#include "GlFrameCompositor.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlStateStack.h"
#include "GlTexture.h"
#include "GlShaderCache.h"
#include "GlTextRenderer.h"
#include "GlLineRenderer.h"
#include "GlViewport.h"
#include "GlModelResourceManager.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "MikanServer.h"
#include "SdlWindow.h"
#include "StringUtils.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"
#include "RmlManager.h"

#include <algorithm>

#include <easy/profiler.h>

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

//-- constants -----
static const int k_window_pixel_width = 1280 + 350;
static const int k_window_pixel_height = 720 + 45;

static const glm::vec4 k_clear_color = glm::vec4(0.45f, 0.45f, 0.5f, 1.f);

static const glm::vec3 k_frustum_color = glm::vec3(0.1f, 0.7f, 0.3f);

//-- statics -----
MainWindow* MainWindow::m_instance = NULL;

//-- public methods -----
MainWindow::MainWindow()
	: m_sdlWindow(SdlWindowUniquePtr(new SdlWindow))
	, m_glStateStack(GlStateStackUniquePtr(new GlStateStack))
	, m_lineRenderer(GlLineRendererUniquePtr(new GlLineRenderer))
	, m_textRenderer(GlTextRendererUniquePtr(new GlTextRenderer))
	, m_modelResourceManager(GlModelResourceManagerUniquePtr(new GlModelResourceManager))
	, m_isRenderingStage(false)
	, m_isRenderingUI(false)
	, m_shaderCache(GlShaderCacheUniquePtr(new GlShaderCache))
{}

MainWindow::~MainWindow()
{
	assert(m_instance == nullptr);
	assert(m_textRenderer == nullptr);
	assert(m_lineRenderer == nullptr);
}

GlLineRenderer* MainWindow::getLineRenderer()
{
	return m_lineRenderer.get();
}

GlTextRenderer* MainWindow::getTextRenderer()
{
	return m_textRenderer.get();
}

GlShaderCache* MainWindow::getShaderCache()
{
	return m_shaderCache.get();
}

GlModelResourceManager* MainWindow::getModelResourceManager()
{
	return m_modelResourceManager.get();
}

SdlWindow& MainWindow::getSdlWindow()
{
	return *m_sdlWindow.get();
}

GlViewportConstPtr MainWindow::getRenderingViewport() const
{
	return m_renderingViewport;
}
GlStateStack& MainWindow::getGlStateStack()
{
	return *m_glStateStack.get();
}

bool MainWindow::startup()
{
	EASY_FUNCTION();

	bool success = true;

	MIKAN_LOG_INFO("MainWindow::init()") << "Initializing MainWindow";
	m_instance = this;

	auto windowTitle= StringUtils::stringify("MikanXR v", MIKAN_RELEASE_VERSION_STRING);
	m_sdlWindow
		->setTitle(windowTitle)
		->setSize(k_window_pixel_width, k_window_pixel_height);
	if (!m_sdlWindow->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Unable to initialize main SDK window: ";
		success = false;
	}

	if (success && !m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Failed to initialize shader cache!";
		success = false;
	}

	if (success)
	{
		if (!m_modelResourceManager->startup())
		{
			MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize model resource manager";
			success = false;
		}
	}

	if (success)
	{
		if (!m_lineRenderer->startup())
		{
			MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize line renderer";
			success = false;
		}
	}

	if (success)
	{
		m_rmlUiRenderer = GlRmlUiRenderUniquePtr(new GlRmlUiRender(*this));
		if (!m_rmlUiRenderer->startup())
		{
			MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize RmlUi Renderer";
			success = false;
		}
	}

	if (success)
	{
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

		// Create a fullscreen viewport for the UI (which creates it's own camera)
		m_uiViewport = 
			std::make_shared<GlViewport>(
				glm::i32vec2(k_window_pixel_width, k_window_pixel_height));
	}

	return success;
}

void MainWindow::update(float deltaSeconds)
{
	App* app= App::getInstance();

	// Update all connected devices	
	app->getVideoSourceManager()->update(deltaSeconds);
	app->getVRDeviceManager()->update(deltaSeconds);

	// Poll rendered frames from client connections
	app->getMikanServer()->update();

	// Update any frame compositing state based on new video frames or client render target updates
	app->getFrameCompositor()->update(deltaSeconds);

	// Garbage collect stale baked text
	app->getFontManager()->garbageCollect();

	// Process any pending app stage operations queued by pushAppStage/popAppStage from last frame
	app->processPendingAppStageOps();

	// Update the current app stage last
	AppStage* appStage = app->getCurrentAppStage();
	if (appStage != nullptr && appStage->getIsUpdateActive())
	{
		EASY_BLOCK("appStage Update");
		appStage->update(deltaSeconds);
	}

	// Update the UI layout and data models
	app->getRmlManager()->update();
}

void MainWindow::render()
{
	App* app= App::getInstance();
	AppStage* appStage= app->getCurrentAppStage();

	if (appStage != nullptr)
	{
		renderBegin();

		// Render all 3d viewports for the app state
		for (GlViewportPtr viewpoint : appStage->getViewportList())
		{
			EASY_BLOCK("appStage render");

			renderStageBegin(viewpoint);
			appStage->render();
			renderStageEnd();
		}

		// Render the UI on top
		{
			EASY_BLOCK("appStage renderUI");
			renderUIBegin();

			appStage->renderUI();

			// Always draw the FPS in the lower right
			TextStyle style = getDefaultTextStyle();
			style.horizontalAlignment = eHorizontalTextAlignment::Right;
			style.verticalAlignment = eVerticalTextAlignment::Bottom;
			drawTextAtScreenPosition(
				style,
				glm::vec2(getWidth() - 1, getHeight() - 1),
				L"%.1ffps", app->getFPS());

			renderUIEnd();
		}

		renderEnd();
	}
}

void MainWindow::shutdown()
{
	m_uiViewport = nullptr;

	m_glStateStack= nullptr;

	if (m_rmlUiRenderer != nullptr)
	{
		m_rmlUiRenderer->shutdown();
		m_rmlUiRenderer= nullptr;
	}

	if (m_modelResourceManager != nullptr)
	{
		m_modelResourceManager->shutdown();
		m_modelResourceManager= nullptr;
	}

	m_textRenderer= nullptr;
	m_lineRenderer= nullptr;

	if (m_shaderCache != nullptr)
	{
		m_shaderCache->shutdown();
		m_shaderCache= nullptr;
	}

	if (m_sdlWindow != nullptr)
	{
		m_sdlWindow->shutdown();
		m_sdlWindow = nullptr;
	}

	m_instance = NULL;
}

float MainWindow::getWidth() const
{
	return (float)m_sdlWindow->getWidth();
}

float MainWindow::getHeight() const
{
	return (float)m_sdlWindow->getHeight();
}

float MainWindow::getAspectRatio() const
{
	return (float)m_sdlWindow->getAspectRatio();
}

void MainWindow::renderBegin()
{
	m_sdlWindow->renderBegin();
}

bool MainWindow::onSDLEvent(const SDL_Event* event)
{
	m_sdlWindow->onSDLEvent(event);

	return m_rmlUiRenderer->onSDLEvent(event);
}

void MainWindow::renderStageBegin(GlViewportConstPtr targetViewport)
{
	EASY_FUNCTION();

	m_renderingViewport = targetViewport;
	m_renderingViewport->applyViewport();

	m_isRenderingStage = true;
}

void MainWindow::renderStageEnd()
{
	EASY_FUNCTION();

	// Render any line segments emitted by the AppStage
	m_lineRenderer->render(this);

	// Render any glyphs emitted by the AppStage
	m_textRenderer->render(this);

	m_renderingViewport = nullptr;
	m_isRenderingStage = false;
}

void MainWindow::renderUIBegin()
{
	EASY_FUNCTION();

	m_renderingViewport = m_uiViewport;
	m_renderingViewport->applyViewport();

	m_rmlUiRenderer->beginFrame();

	m_isRenderingUI = true;
}

void MainWindow::renderUIEnd()
{
	EASY_FUNCTION();

	m_rmlUiRenderer->endFrame();

	// Render any line segments emitted by the AppStage renderUI phase
	m_lineRenderer->render(this);

	// Render any glyphs emitted by the AppStage renderUI phase
	m_textRenderer->render(this);

	m_renderingViewport = nullptr;
	m_isRenderingUI = false;
}

void MainWindow::renderEnd()
{
	m_sdlWindow->renderEnd();
}