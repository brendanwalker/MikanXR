//-- includes -----
#include "App.h"
#include "AppStage.h"
#include "AppStage_MainMenu.h"
#include "FontManager.h"
#include "FrameTimer.h"
#include "GlShaderCache.h"
#include "GlFrameCompositor.h"
#include "GlTextRenderer.h"
#include "InputManager.h"
#include "LocalizationManager.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "Logger.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"

#include "SDL_timer.h"
#include "SDL_clipboard.h"

#include <easy/profiler.h>

#include <RmlUi/Core.h>

#ifdef _WIN32
#include "Objbase.h"
#endif //_WIN32

//-- static members -----
App* App::m_instance= nullptr;

//-- public methods -----
App::App()
	: m_profileConfig(new ProfileConfig())
	, m_mikanServer(new MikanServer())
	, m_frameCompositor(new GlFrameCompositor())
	, m_inputManager(new InputManager())
	, m_localizationManager(new LocalizationManager())
	, m_renderer(new Renderer())
	, m_shaderCache(new GlShaderCache())
	, m_fontManager(new FontManager())
	, m_videoSourceManager(new VideoSourceManager())
	, m_vrDeviceManager(new VRDeviceManager())
	, m_bShutdownRequested(false)
{
	m_instance= this;
}

App::~App()
{
	shutdown();

	delete m_vrDeviceManager;
	delete m_videoSourceManager;
	delete m_shaderCache;
	delete m_renderer;	
	delete m_localizationManager;
	delete m_inputManager;
	delete m_mikanServer;
	delete m_frameCompositor;
	delete m_profileConfig;

	m_instance= nullptr;
}

int App::exec(int argc, char** argv)
{
	int result = 0;

	if (startup(argc, argv))
	{
		SDL_Event e;

		pushAppStage<AppStage_MainMenu>();

		while (!m_bShutdownRequested)
		{
			FrameTimer frameTimer(11); // 11ms = 90fps

			if (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT ||
					(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
				{
					MIKAN_LOG_INFO("App::exec") << "QUIT message received";
					break;
				}
				else
				{
					onSDLEvent(e);
				}
			}

			update();
			render();

			frameTimer.waitForNextFrame();
		}
	}
	else
	{
		MIKAN_LOG_ERROR("App::exec") << "Failed to initialize application!";
		result = -1;
	}

	shutdown();

	return result;
}

//-- private methods -----
bool App::startup(int argc, char** argv)
{
	bool success = true;

	LoggerSettings settings = {};
	settings.min_log_level = LogSeverityLevel::debug;
	settings.enable_console= true;
	settings.log_filename= "MikanXR.log";

	log_init(settings);

	profiler::startListen();

#ifdef _WIN32
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (!SUCCEEDED(hr))
	{
		MIKAN_LOG_ERROR("App::init") << "Could not initialize COM";
		success = false;
	}
#endif // _WIN32

	// Tell the UI libary this class implements the RML System Interface
	Rml::SetSystemInterface(this);

	// Load any saved config
	m_profileConfig->load();

	if (success && !m_localizationManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize localization manager!";
		success = false;
	}

	if (success && !m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize asset manager!";
		success = false;
	}

	if (success && !m_renderer->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize renderer!";
		success = false;
	}

	if (success && !m_fontManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize baked text cache!";
		success = false;
	}

	if (success && !m_videoSourceManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the video source manager";
		success = false;
	}

	if (success && !m_vrDeviceManager->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the vr tracker manager";
		success = false;
	}

	if (success && !m_frameCompositor->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the frame compositor";
		success = false;
	}

	if (success && !m_mikanServer->startup())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the MikanXR server";
		success = false;
	}

	if (success)
	{
		m_lastFrameTimestamp= SDL_GetTicks();
	}

	if (success && !Rml::Initialise())
	{
		MIKAN_LOG_ERROR("App::init") << "Failed to initialize the RmlUi";
		success = false;
	}

	return success;
}

void App::shutdown()
{
	while (getCurrentAppStage() != nullptr)
	{
		popAppState();
	}

	if (m_mikanServer != nullptr)
	{
		m_mikanServer->shutdown();
	}

	if (m_frameCompositor != nullptr)
	{
		m_frameCompositor->shutdown();
	}

	if (m_videoSourceManager != nullptr)
	{
		m_videoSourceManager->shutdown();
	}

	if (m_vrDeviceManager != nullptr)
	{
		m_vrDeviceManager->shutdown();
	}

	if (m_fontManager != nullptr)
	{
		m_fontManager->shutdown();
	}

	if (m_shaderCache != nullptr)
	{
		m_shaderCache->shutdown();
	}

	if (m_renderer != nullptr)
	{
		m_renderer->shutdown();
	}

	if (m_localizationManager != nullptr)
	{
		m_localizationManager->shutdown();
	}

#ifdef _WIN32
	CoUninitialize();
#endif // _WIN32
}

void App::onSDLEvent(SDL_Event& e)
{
	m_inputManager->onSDLEvent(e);
	m_renderer->onSDLEvent(&e);

	AppStage *appStage= getCurrentAppStage();
	if (appStage != nullptr)
	{
		appStage->onSDLEvent(&e);
	}
}

void App::update()
{
	EASY_FUNCTION();

	// Update the frame rate
	const uint32_t now = SDL_GetTicks();
	const float deltaSeconds= (float)(now - m_lastFrameTimestamp) / 1000.f;
	m_fps= deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
	m_lastFrameTimestamp= now;

	// Update all connected devices
	m_videoSourceManager->update(deltaSeconds);
	m_vrDeviceManager->update(deltaSeconds);

	// Poll rendered frames from client connections
	m_mikanServer->update();

	// Update any frame compositing state based on new video frames or client render target updates
	m_frameCompositor->update();

	// Garbage collect stale baked text
	m_fontManager->garbageCollect();

	// Disallow app stack operations during enter or exit
	bAppStackOperationAllowed = false;

	// Process any pending app stage operations queued by pushAppStage/popAppStage from last frame
	InputManager* inputManager= InputManager::getInstance();
	for (auto& pendingAppStageOp : m_pendingAppStageOps)
	{
		switch (pendingAppStageOp.op)
		{
		case AppStageOperation::enter:
			{
				EASY_BLOCK("appStage Enter");

				// Pause the parent app stage
				if (pendingAppStageOp.parentAppStage != nullptr)
					pendingAppStageOp.parentAppStage->pause();

				// Create a new input event set for the app state
				inputManager->pushEventBindingSet();

				// Enter the new app stage
				pendingAppStageOp.appStage->enter();
			} break;
		case AppStageOperation::exit:
			{
				EASY_BLOCK("appStage Exit");

				// Exit the app stage we are leaving
				pendingAppStageOp.appStage->exit();

				// Clean up the input event set for the deactivated app stage
				inputManager->popEventBindingSet();

				// Resume the parent app stage we are restoring
				pendingAppStageOp.parentAppStage->resume();				

				// Free the app state
				delete pendingAppStageOp.appStage;
			} break;
		}
	}
	m_pendingAppStageOps.clear();

	// App stack operations allowed during update
	bAppStackOperationAllowed = true;

	// Update the current app stage last
	AppStage* appStage = getCurrentAppStage();
	if (appStage != nullptr)
	{
		EASY_BLOCK("appStage Update");
		appStage->update();
	}
}

void App::render()
{
	EASY_FUNCTION();

	AppStage* appStage = getCurrentAppStage();

	m_renderer->renderBegin();

	m_renderer->renderStageBegin();
	if (appStage != nullptr)
	{
		EASY_BLOCK("appStage render");

		appStage->render();

		// Draw shared app rendering
		TextStyle style= getDefaultTextStyle();
		style.horizontalAlignment= eHorizontalTextAlignment::Right;
		style.verticalAlignment= eVerticalTextAlignment::Bottom;
		drawTextAtScreenPosition(
			style, 
			glm::vec2(m_renderer->getSDLWindowWidth() - 1, m_renderer->getSDLWindowHeight() - 1),
			L"%.1ffps", m_fps);
	}
	m_renderer->renderStageEnd();

	m_renderer->renderUIBegin();
	if (appStage != nullptr)
	{
		EASY_BLOCK("appStage renderUI");

		appStage->renderUI();
	}
	m_renderer->renderUIEnd();

	m_renderer->renderEnd();
}

// Rml::SystemInterface
double App::GetElapsedTime()
{
	return double(SDL_GetTicks()) / 1000.0;
}

int App::TranslateString(Rml::String& translated, const Rml::String& input)
{
	const std::string& contextName= Renderer::getInstance()->getRmlContextName();

	bool bHasString= false;
	const char* result= locTextUTF8(contextName.c_str(), input.c_str(), &bHasString);

	if (bHasString)
	{
		translated = result;
		return 1;
	}
	else
	{
		translated= input;
		return 0;
	}
}

bool App::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	switch (type)
	{
	case Rml::Log::LT_ASSERT:
		MIKAN_LOG_FATAL("Rml::LogMessage") << message;
		return true;
	case Rml::Log::LT_ALWAYS:
	case Rml::Log::LT_ERROR:
		MIKAN_LOG_ERROR("Rml::LogMessage") << message;
		return true;
	case Rml::Log::LT_WARNING:
		MIKAN_LOG_WARNING("Rml::LogMessage") << message;
		return true;
	case Rml::Log::LT_INFO:
		MIKAN_LOG_INFO("Rml::LogMessage") << message;
		return true;
	case Rml::Log::LT_DEBUG:
		MIKAN_LOG_DEBUG("Rml::LogMessage") << message;
		return true;
	}

	return false;
}

void App::SetMouseCursor(const Rml::String& cursor_name)
{
	m_renderer->setSDLMouseCursor(cursor_name);
}

void App::SetClipboardText(const Rml::String& text_utf8)
{
	SDL_SetClipboardText(text_utf8.c_str());
}

void App::GetClipboardText(Rml::String& text)
{
	char* raw_text = SDL_GetClipboardText();
	text = Rml::String(raw_text);
	SDL_free(raw_text);
}

