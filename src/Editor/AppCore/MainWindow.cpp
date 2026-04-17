//-- includes -----
#include "MainWindow.h"
#include "Logger.h"
#include "Version.h"

#include "App.h"
#include "AppStage.h"
#include "AnchorObjectSystem.h"
#include "ClientSourceManager.h"
#include "EditorObjectSystem.h"
#include "InputManager.h"
#include "IMkGraphicsContext.h"
#include "IMkWindow.h"
#include "IMkState.h"
#include "IMkTexture.h"
#include "IMkTextRenderer.h"
#include "IMkLineRenderer.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "MikanCamera.h"
#include "IMkFontManager.h"
#include "MikanServer.h"
#include "MikanViewport.h"
#include "MikanModelResourceManager.h"
#include "MkGuiContext.h"
#include "MkGuiScopedUpdate.h"
#include "MkGuiStyleManager.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "MikanTextRenderer.h"
#include "MkWindowEvent.h"
#include "PathUtils.h"
#include "ProjectManager.h"
#include "OpenCVManager.h"
#include "StencilUtils.h"
#include "StringUtils.h"
#include "TextStyle.h"

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
	, m_inputManager(new InputManager(this))
	, m_projectManager(std::make_shared<ProjectManager>(this))
	, m_openCVManager(new OpenCVManager())
	, m_fontManager(createMkFontManager())
	, m_appStageFactory(this)
	, m_graphicsContext(createMkGraphicsContext(eGraphicsAPI::OpenGL, m_fontManager.get()))
	, m_mkWindow(createMkWindow(m_ownerApp->getWindowManager(), m_graphicsContext))
	, m_modelResourceManager(MikanModelResourceManagerUniquePtr(new MikanModelResourceManager(this)))
	, m_isRenderingStage(false)
	, m_isRenderingUI(false)
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
	delete m_mikanServer;
	delete m_clientSourceManager;
}

MikanModelResourceManager* MainWindow::getModelResourceManager()
{
	return m_modelResourceManager.get();
}

void MainWindow::getMouseScreenPosition(int& outScreenX, int& outScreenY) const
{
	m_mkWindow->getMouseScreenPosition(outScreenX, outScreenY);
}

eWindowAPI MainWindow::getWindowAPI() const
{
	return m_mkWindow->getWindowAPI();
}

void* MainWindow::getNativeWindowHandle() const
{
	return m_mkWindow->getNativeWindowHandle();
}

IMkGraphicsContextPtr MainWindow::getGraphicsContext() const
{
	return m_graphicsContext;
}

void MainWindow::makeContextCurrent()
{
	m_mkWindow->makeContextCurrent();
}

void MainWindow::present()
{
	m_mkWindow->present();
}

bool MainWindow::wantsDestroy() const
{
	return m_mkWindow->wantsDestroy();
}

void MainWindow::setTitle(const std::string& title)
{
	m_mkWindow->setTitle(title);
}

void MainWindow::setSize(int width, int height)
{
	m_mkWindow->setSize(width, height);
}

void MainWindow::handleEvents(IMkWindowEventListener* eventListener)
{
	m_mkWindow->handleEvents(eventListener);
}

bool MainWindow::hasMouseFocus() const
{
	return m_mkWindow->hasMouseFocus();
}

bool MainWindow::hasKeyboardFocus() const
{
	return m_mkWindow->hasKeyboardFocus();
}

EventBus* MainWindow::getEventBus() const
{
	return m_ownerApp->getEventBus();
}

class MkGuiStyleManager* MainWindow::getMkGuiStyleManager() const
{
	return m_styleManager.get();
}

LocalizationManager* MainWindow::getLocalizationManager() const
{
	return m_ownerApp->getLocalizationManager();
}

IMkViewportPtr MainWindow::getRenderingViewport() const
{
	return m_renderingViewport;
}

bool MainWindow::startup()
{
	EASY_FUNCTION();

	bool success = true;

	MIKAN_LOG_INFO("MainWindow::init()") << "Initializing MainWindow";

	auto windowTitle= StringUtils::stringify("MikanXR v", MIKAN_RELEASE_VERSION_STRING);
	m_mkWindow->setTitle(windowTitle);
	m_mkWindow->setSize(k_window_pixel_width, k_window_pixel_height);
	if (!m_mkWindow->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::startup") << "Unable to initialize main SDK window: ";
		success = false;
	}

	if (success)
	{
		m_guiContext = std::make_shared<MkGuiContext>(this);
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

	if (success && !m_modelResourceManager->startup())
	{
		MIKAN_LOG_ERROR("MainWindow::init") << "Unable to initialize model resource manager";
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
		// Create the base GL state for the window
		IMkState* mkState= m_graphicsContext->getMkStateStack().pushState("MainWindow Root");
		assert(mkState->getStackDepth() == 0);

		// Set default state flags at the base of the stack
		mkState->disableFlag(eMkStateFlagType::cullFace);

		// Set the default clear color
		mkStateSetClearColor(mkState, k_clear_color);

		// Default to the full window viewport
		mkStateSetViewport(mkState, 0, 0, m_mkWindow->getWidth(), m_mkWindow->getHeight());

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
	m_mkWindow->handleEvents(this);

	// Update objects in the object system
	m_projectManager->update(deltaSeconds);

	// Update the current app stage last
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr && appStage->getIsUpdateActive())
	{
		// Process input events in the Debug UI and render the UI
		if (m_bIsDebugGuiEnabled)
		{
			EASY_BLOCK("appStage onGui");
			appStage->onGui();
		}

		// Update the simulation of the current app stage
		{
			EASY_BLOCK("appStage Update");
			appStage->update(deltaSeconds);
		}
	}
}

void MainWindow::render()
{
	AppStage* appStage= getCurrentAppStage();

	if (appStage != nullptr)
	{
		// Clear the window
		m_graphicsContext->renderBegin();

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

		// Finalize rendering
		m_graphicsContext->renderEnd();

		// Present the rendered frame to the window
		m_mkWindow->present();
	}
}

void MainWindow::renderStageViewport(AppStage* appStage, IMkViewportPtr targetViewport)
{
	EASY_FUNCTION();

	MkScopedState scopedState = m_graphicsContext->getMkStateStack().createScopedState("appStage viewport render");
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
	m_graphicsContext->getLineRenderer()->render();

	// Render any glyphs emitted by the AppStage
	m_graphicsContext->getTextRenderer()->render();

	// Rendering the state is done
	m_isRenderingStage = false;

	// Forget about the target viewport
	// (will be deleted when glState goes out of scope)
	m_renderingViewport = nullptr;
}

void MainWindow::renderStageUI(AppStage* appStage)
{
	EASY_FUNCTION();

	MkScopedState scopedState = m_graphicsContext->getMkStateStack().createScopedState("appStage renderUI");
	IMkState* glState = scopedState.getStackState();

	// Set the rendering viewport used to render the stage
	// (adds mkStateSetViewport Modifier to the glState)
	m_renderingViewport = m_uiViewport;
	m_renderingViewport->applyRenderingViewport(glState);

	m_isRenderingUI = true;

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
	m_graphicsContext->getLineRenderer()->render();

	// Render any glyphs emitted by the AppStage renderUI phase
	m_graphicsContext->getTextRenderer()->render();

	m_isRenderingUI = false;

	m_renderingViewport = nullptr;
}

void MainWindow::shutdown()
{
	m_uiViewport = nullptr;

	// Tear down all active app stages
	while (getCurrentAppStage() != nullptr)
	{
		popAppState();
	}
	processPendingAppStageOps();

	assert(m_mikanServer != nullptr);
	m_mikanServer->shutdown();

	assert(m_clientSourceManager != nullptr);
	m_clientSourceManager->shutdown();

	// Dispose all ObjectSystems
	assert(m_projectManager != nullptr);
	m_projectManager->shutdown();

	assert(m_fontManager != nullptr);
	m_fontManager->shutdown();

	if (m_modelResourceManager != nullptr)
	{
		m_modelResourceManager->shutdown();
		m_modelResourceManager= nullptr;
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

	if (m_mkWindow != nullptr)
	{
		m_mkWindow->shutdown();
		m_mkWindow = nullptr;
	}
}

const char* MainWindow::getTitle() const
{
	return m_mkWindow->getTitle();
}

float MainWindow::getWidth() const
{
	return m_mkWindow->getWidth();
}

float MainWindow::getHeight() const
{
	return m_mkWindow->getHeight();
}

float MainWindow::getAspectRatio() const
{
	return m_mkWindow->getAspectRatio();
}

bool MainWindow::onWindowEvent(const MkWindowEvent& event)
{
	bool bHandled = false;

	const auto eventType = event.getEventType();
	const auto keySym = event.getKeySym();

	// First see if we got an app shutdown request
	if (eventType == eMkWindowEventType::Quit ||
		(eventType == eMkWindowEventType::KeyDown && keySym == MkKey::ESCAPE))
	{
		MIKAN_LOG_INFO("App::exec") << "QUIT message received";
		App::getInstance()->requestShutdown();
		bHandled = true;
	}
	// Toggle debug UI with F11
	else if (eventType == eMkWindowEventType::KeyUp && keySym == MkKey::F11)
	{
		m_bIsDebugGuiEnabled = !m_bIsDebugGuiEnabled;
	}

	// Then see if the UI wants to handle the event
	if (!bHandled && m_bIsDebugGuiEnabled)
	{
		bHandled = m_guiContext->onWindowEvent(event);
	}

	// Then see if the current app stage wants to handle the event
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr)
	{
		appStage->onWindowEvent(event);
	}

	// Then see if the main window object simulation wants to handle the event
	if (!bHandled)
	{
		if (m_mkWindow->hasMouseFocus() || m_mkWindow->hasKeyboardFocus())
		{
			bHandled = m_inputManager->onWindowEvent(event);
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
					m_inputManager->pushEventBindingSet();

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
					m_inputManager->popEventBindingSet();

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