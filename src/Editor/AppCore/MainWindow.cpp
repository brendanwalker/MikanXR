//-- includes -----
#include "MainWindow.h"
#include "Logger.h"
#include "Version.h"

#include "GlRmlUiRenderer.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include "App.h"
#include "AppStage.h"
#include "AnchorObjectSystem.h"
#include "ClientSourceManager.h"
#include "EditorObjectSystem.h"
#include "InputManager.h"
#include "SdlCommon.h"
#include "IMkState.h"
#include "IMkTexture.h"
#include "IMkTextRenderer.h"
#include "IMkLineRenderer.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "MikanCamera.h"
#include "MikanFontManager.h"
#include "MikanShaderCache.h"
#include "MikanServer.h"
#include "MikanTextureCache.h"
#include "MikanTextRenderer.h"
#include "MikanViewport.h"
#include "MikanModelResourceManager.h"
#include "MkGuiContext.h"
#include "MkGuiScopedUpdate.h"
#include "MkGuiStyleManager.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "PathUtils.h"
#include "ProjectManager.h"
#include "OpenCVManager.h"
#include "RmlManager.h"
#include "SdlWindow.h"
#include "StencilUtils.h"
#include "StringUtils.h"

// App Stages
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "Project/AppStage_Project.h"
#include "StencilAlignment/AppStage_StencilAlignment.h"
#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"
#include "VideoSourceSettings/AppStage_VideoSourceSettings.h"
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"

#include <algorithm>

#include <easy/profiler.h>

//-- constants -----
static const int k_window_pixel_width = 1280;
static const int k_window_pixel_height = 720;

static const glm::vec4 k_clear_color = glm::vec4(0.45f, 0.45f, 0.5f, 1.f);

static const glm::vec3 k_frustum_color = glm::vec3(0.1f, 0.7f, 0.3f);

//-- public methods -----
MainWindow::MainWindow(App* ownerApp)
	: m_ownerApp(ownerApp)
	, m_mikanServer(new MikanServer())
	, m_clientSourceManager(new ClientSourceManager())
	, m_inputManager(new InputManager())
	, m_rmlManager(new RmlManager(this))
	, m_projectManager(std::make_shared<ProjectManager>(this))
	, m_openCVManager(new OpenCVManager())
	, m_fontManager(new MikanFontManager())
	, m_appStageFactory(this)
	, m_sdlWindow(SdlWindowUniquePtr(new SdlWindow(this)))
	, m_mkStateStack(MkStateStackUniquePtr(new MkStateStack(this)))
	, m_lineRenderer(createMkLineRenderer(this))
	, m_textRenderer(createMkTextRenderer(this, m_fontManager))
	, m_modelResourceManager(MikanModelResourceManagerUniquePtr(new MikanModelResourceManager(this)))
	, m_isRenderingStage(false)
	, m_isRenderingUI(false)
	, m_shaderCache(MikanShaderCacheUniquePtr(new MikanShaderCache(this)))
	, m_textureCache(MikanTextureCacheUniquePtr(new MikanTextureCache(this)))
{
	m_appStageFactory.addAppStageConstructor<AppStage_AlignmentCalibration>();
	m_appStageFactory.addAppStageConstructor<AppStage_AnchorTriangulation>();
	m_appStageFactory.addAppStageConstructor<AppStage_MainMenu>();
	m_appStageFactory.addAppStageConstructor<AppStage_MonoLensCalibration>();
	m_appStageFactory.addAppStageConstructor<AppStage_Project>();
	m_appStageFactory.addAppStageConstructor<AppStage_StencilAlignment>();
	m_appStageFactory.addAppStageConstructor<AppStage_TextureSourceSettings>();
	m_appStageFactory.addAppStageConstructor<AppStage_VideoSourceSettings>();
	m_appStageFactory.addAppStageConstructor<AppStage_VRTrackingRecenter>();
}

MainWindow::~MainWindow()
{
	m_projectManager = nullptr;
	delete m_openCVManager;
	delete m_inputManager;
	delete m_rmlManager;
	delete m_mikanServer;
	delete m_clientSourceManager;

	assert(m_textRenderer == nullptr);
	assert(m_lineRenderer == nullptr);
}

IMkLineRenderer* MainWindow::getLineRenderer()
{
	return m_lineRenderer.get();
}

IMkTextRenderer* MainWindow::getTextRenderer()
{
	return m_textRenderer.get();
}

IMkShaderCache* MainWindow::getShaderCache()
{
	return m_shaderCache.get();
}

IMkTextureCache* MainWindow::getTextureCache()
{
	return m_textureCache.get();
}

MikanModelResourceManager* MainWindow::getModelResourceManager()
{
	return m_modelResourceManager.get();
}

SdlWindow& MainWindow::getSdlWindow()
{
	return *m_sdlWindow.get();
}

EventBus* MainWindow::getEventBus() const
{
	return m_ownerApp->getEventBus();
}

LocalizationManager* MainWindow::getLocalizationManager() const
{
	return m_ownerApp->getLocalizationManager();
}

IMkViewportPtr MainWindow::getRenderingViewport() const
{
	return m_renderingViewport;
}
MkStateStack& MainWindow::getMkStateStack()
{
	return *m_mkStateStack.get();
}

bool MainWindow::startup()
{
	EASY_FUNCTION();

	bool success = true;

	MIKAN_LOG_INFO("MainWindow::init()") << "Initializing MainWindow";

	if (success && !m_rmlManager->preRendererStartup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Rml UI manager!";
		success = false;
	}

	auto windowTitle= StringUtils::stringify("MikanXR v", MIKAN_RELEASE_VERSION_STRING);
	m_sdlWindow
		->setTitle(windowTitle)
		->setSize(k_window_pixel_width, k_window_pixel_height);
	if (!m_sdlWindow->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Unable to initialize main SDK window: ";
		success = false;
	}

	if (success)
	{
		m_guiContext = std::make_shared<MkGuiContext>(
			m_sdlWindow->getInternalSdlWindow(),
			m_sdlWindow->getInternalGlContext());
		if (!m_guiContext->startup())
		{
			MIKAN_LOG_ERROR("NodeEditorWindow::startup") << "Unable to create GUI context";
			success = false;
		}
	}

	if (success)
	{
		m_styleManager = std::make_unique<MkGuiStyleManager>();
		const auto stylesPath = PathUtils::getResourceDirectory() / "gui_styles";
		if (!m_styleManager->startup(m_guiContext.get(), stylesPath))
		{
			MIKAN_LOG_ERROR("NodeEditorWindow::startup") << "Failed to initialize style manager";
			success = false;
		}
	}

	if (success && !m_openCVManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize OpenCV manager!";
		success = false;
	}

	if (success && !m_textureCache->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Failed to initialize texture cache!";
		success = false;
	}

	if (success && !m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Failed to initialize shader cache!";
		success = false;
	}

	if (success && !m_modelResourceManager->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize model resource manager";
		success = false;
	}

	if (success && !m_lineRenderer->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize line renderer";
		success = false;
	}

	if (success && !m_textRenderer->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize line renderer";
		success = false;
	}

	if (success && !m_fontManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize baked text cache!";
		success = false;
	}

	if (success)
	{
		if (!m_projectManager->startup(this))
		{			
			MIKAN_LOG_ERROR("App::init") << "Failed to initialize the object system manager";
			success = false;
		}
	}

	if (success && !m_clientSourceManager->startup(this))
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the client source manager";
		success = false;
	}

	if (success && !m_mikanServer->startup(this))
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the MikanXR server";
		success = false;
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

	if (success && !m_rmlManager->postRendererStartup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize Rml UI manager!";
		success = false;
	}

	if (success)
	{
		// Create the base GL state for the window
		IMkState* mkState= m_mkStateStack->pushState("MainWindow Root");
		assert(mkState->getStackDepth() == 0);

		// Set default state flags at the base of the stack
		mkState->disableFlag(eMkStateFlagType::cullFace);

		// Set the default clear color
		mkStateSetClearColor(mkState, k_clear_color);

		// Default to the full window viewport
		mkStateSetViewport(mkState, 0, 0, m_sdlWindow->getWidth(), m_sdlWindow->getHeight());

		// Create a fullscreen viewport for the UI (which creates it's own camera)
		m_uiViewport = 
			std::make_shared<MikanViewport>(
				this,
				glm::i32vec2(k_window_pixel_width, k_window_pixel_height));
	}

	if (success)
	{
		pushAppStageOfType<AppStage_MainMenu>();
	}

	return success;
}

void MainWindow::update(float deltaSeconds)
{
	// Push the ImGui Update scope
	MkGuiScopedUpdate mkScopedCtx(*m_guiContext);

	// Poll rendered frames from client connections
	m_mikanServer->update();

	// Garbage collect stale baked text
	m_fontManager->garbageCollect();

	// Process any pending app stage operations queued by pushAppStage/popAppStage from last frame
	processPendingAppStageOps();

	// Process most recent SDL events (keyboard, mouse, etc)
	m_sdlWindow->handleSDLEvents(this);

	// Update objects in the object system
	m_projectManager->update(deltaSeconds);

	// Update the current app stage last
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr && appStage->getIsUpdateActive())
	{
		// Update the simulation of the current app stage
		{
			EASY_BLOCK("appStage Update");
			appStage->update(deltaSeconds);
		}

		// Update the UI for the current app stage
		if (m_bIsDebugGuiEnabled)
		{
			EASY_BLOCK("appStage onGui");
			appStage->onGui();
		}
	}

	// Update the UI layout and data models
	m_rmlManager->update();
}

void MainWindow::render()
{
	AppStage* appStage= getCurrentAppStage();

	if (appStage != nullptr)
	{
		m_sdlWindow->renderBegin();

		// Render all enabled 3d viewports for the app state
		for (MikanViewportPtr viewpoint : appStage->getViewportList())
		{
			if (viewpoint->getIsRenderingEnabled())
			{
				renderStageViewport(appStage, viewpoint);
			}
		}

		// Render the UI on top
		renderStageUI(appStage);

		m_sdlWindow->renderEnd();
	}
}

void MainWindow::renderStageViewport(AppStage* appStage, IMkViewportPtr targetViewport)
{
	EASY_FUNCTION();

	MkScopedState scopedState = m_mkStateStack->createScopedState("appStage viewport render");
	IMkState* glState = scopedState.getStackState();

	// Set the rendering viewport used to render the stage
	// (adds mkStateSetViewport Modifier to the glState)
	m_renderingViewport = targetViewport;
	m_renderingViewport->applyRenderingViewport(glState);

	// Set window state flag that we are in the middle of rendering a stage
	// Used for safety checks in the render functions
	m_isRenderingStage = true;

	// Render the 3d geometry of the AppStage
	appStage->render(targetViewport);

	// Render any 3D line segments emitted by the AppStage
	m_lineRenderer->render();

	// Render any glyphs emitted by the AppStage
	m_textRenderer->render();

	// Rendering the state is done
	m_isRenderingStage = false;

	// Forget about the target viewport
	// (will be deleted when glState goes out of scope)
	m_renderingViewport = nullptr;
}

void MainWindow::renderStageUI(AppStage* appStage)
{
	EASY_FUNCTION();

	MkScopedState scopedState = m_mkStateStack->createScopedState("appStage renderUI");
	IMkState* glState = scopedState.getStackState();

	// Set the rendering viewport used to render the stage
	// (adds mkStateSetViewport Modifier to the glState)
	m_renderingViewport = m_uiViewport;
	m_renderingViewport->applyRenderingViewport(glState);

	m_isRenderingUI = true;

	// Render the UI of the AppStage
	appStage->renderUI();

	// Submit the MkGui draw call
	m_guiContext->submitDrawData();

	// Always draw the FPS in the lower right
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Right;
	style.verticalAlignment = eVerticalTextAlignment::Bottom;
	drawTextAtScreenPosition(
		style,
		glm::vec2(getWidth() - 1, getHeight() - 1),
		L"%.1ffps", App::getInstance()->getFPS());

	// Render any 2D line segments emitted by the AppStage renderUI phase
	m_lineRenderer->render();

	// Render any glyphs emitted by the AppStage renderUI phase
	m_textRenderer->render();

	m_isRenderingUI = false;

	m_renderingViewport = nullptr;
}

void MainWindow::shutdown()
{
	m_uiViewport = nullptr;
	m_mkStateStack= nullptr;

	// Tear down all active app stages
	while (getCurrentAppStage() != nullptr)
	{
		popAppState();
	}
	processPendingAppStageOps();

	// Tear down all app systems
	assert(m_rmlManager != nullptr);
	m_rmlManager->shutdown();

	assert(m_mikanServer != nullptr);
	m_mikanServer->shutdown();

	assert(m_clientSourceManager != nullptr);
	m_clientSourceManager->shutdown();

	// Dispose all ObjectSystems
	assert(m_projectManager != nullptr);
	m_projectManager->shutdown();

	assert(m_fontManager != nullptr);
	m_fontManager->shutdown();

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

	if (m_textureCache != nullptr)
	{
		m_textureCache->shutdown();
		m_textureCache = nullptr;
	}

	if (m_styleManager != nullptr)
	{
		m_styleManager->shutdown();
		m_styleManager = nullptr;
	}

	if (m_guiContext != nullptr)
	{
		m_guiContext->shutdown();
		m_guiContext = nullptr;
	}

	if (m_sdlWindow != nullptr)
	{
		m_sdlWindow->shutdown();
		m_sdlWindow = nullptr;
	}
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

bool MainWindow::onWindowEvent(const SDL_Event* event)
{
	bool bHandled = false;

	// First see if we got an app shutdown request
	if (event->type == SDL_QUIT ||
		(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE))
	{
		MIKAN_LOG_INFO("App::exec") << "QUIT message received";
		App::getInstance()->requestShutdown();
		bHandled= true;
	}
	// Toggle debug UI with F11
	else if (event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_F11)
	{
		m_bIsDebugGuiEnabled = !m_bIsDebugGuiEnabled;
	}

	// Then see if the UI wants to handle the event
	if (!bHandled && m_bIsDebugGuiEnabled)
	{
		bHandled= m_guiContext->onWindowEvent(event);
	}

	// Then see if the UI wants to handle the event
	if (!bHandled)
	{
		bHandled= m_rmlUiRenderer->onSDLEvent(event);
	}

	// Then see if the current app stage wants to handle the event
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr)
	{
		appStage->onSDLEvent(event);
	}

	// Then see if the main window object simulation wants to handle the event
	if (!bHandled)
	{
		SdlWindow& sdlWindow= getSdlWindow();
		
		if (sdlWindow.hasMouseFocus() || sdlWindow.hasKeyboardFocus())
		{
			bHandled = m_inputManager->onSDLEvent(event);
		}
	}

	return bHandled;
}

inline AppStage* MainWindow::getCurrentAppStage() const
{
	return (m_appStageStack.size() > 0) ? m_appStageStack[m_appStageStack.size() - 1].get() : nullptr;
}

inline AppStage* MainWindow::getParentAppStage() const
{
	return (m_appStageStack.size() > 1) ? m_appStageStack[m_appStageStack.size() - 2].get() : nullptr;
}

AppStage* MainWindow::pushAppStage(const std::string& appStageName)
{
	assert(bAppStackOperationAllowed);

	AppStagePtr newAppStage= m_appStageFactory.allocateAppStage(appStageName);
	if (newAppStage)
	{
		AppStagePtr parentAppStage =
			m_appStageStack.size() > 0
			? m_appStageStack[m_appStageStack.size() - 1]
			: AppStagePtr();

		m_appStageStack.push_back(newAppStage);
		m_pendingAppStageOps.push_back({ parentAppStage, newAppStage, AppStageOperation::enter});

		return newAppStage.get();
	}

	return nullptr;
}

void MainWindow::popAppState()
{
	assert(bAppStackOperationAllowed);
	AppStagePtr appStage = 
		m_appStageStack.size() > 0
		? m_appStageStack[m_appStageStack.size() - 1]
		: AppStagePtr();
	if (appStage)
	{
		m_appStageStack.pop_back();

		AppStagePtr parentAppStage =
			m_appStageStack.size() > 0
			? m_appStageStack[m_appStageStack.size() - 1]
			: AppStagePtr();

		m_pendingAppStageOps.push_back({ parentAppStage, appStage, AppStageOperation::exit});
	}
}


void MainWindow::processPendingAppStageOps()
{
	// Disallow app stack operations during enter or exit
	bAppStackOperationAllowed = false;

	InputManager* inputManager = InputManager::getInstance();
	for (auto& pendingAppStageOp : m_pendingAppStageOps)
	{
		switch (pendingAppStageOp.op)
		{
			case AppStageOperation::enter:
				{
					EASY_BLOCK("appStage Enter");

					// Pause the parent app stage
					if (pendingAppStageOp.parentAppStage)
						pendingAppStageOp.parentAppStage->pause();

					// Create a new input event set for the app state
					inputManager->pushEventBindingSet();

					// Enter the new app stage
					pendingAppStageOp.appStage->enter();

					// Notify any object systems that care about app stage transitions 
					if (OnAppStageEntered)
						OnAppStageEntered(pendingAppStageOp.parentAppStage.get(), pendingAppStageOp.appStage.get());
				} break;
			case AppStageOperation::exit:
				{
					EASY_BLOCK("appStage Exit");

					// Notify any object systems that care about app stage transitions 
					if (OnAppStageExited)
						OnAppStageExited(pendingAppStageOp.appStage.get(), pendingAppStageOp.parentAppStage.get());

					// Exit the app stage we are leaving
					pendingAppStageOp.appStage->exit();

					// Clean up the input event set for the deactivated app stage
					inputManager->popEventBindingSet();

					// Resume the parent app stage we are restoring (if any)
					if (pendingAppStageOp.parentAppStage != nullptr)
						pendingAppStageOp.parentAppStage->resume();

					// Free the app state
					pendingAppStageOp.appStage= nullptr;
				} break;
		}
	}
	m_pendingAppStageOps.clear();

	// App stack operations allowed during update
	bAppStackOperationAllowed = true;
}