//-- includes -----
#include "Renderer.h"
#include "Logger.h"
#include "Version.h"

#include "GlRmlUiRenderer.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#if defined(_WIN32)
	#include <SDL.h>
	#include <SDL_events.h>
	#include <SDL_mouse.h>
	#include <SDL_syswm.h>
	#include <SDL_image.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_events.h>
	#include <SDL2/SDL_mouse.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_syswm.h>
#endif

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
Renderer* Renderer::m_instance = NULL;

//-- public methods -----
Renderer::Renderer()
	: m_sdlInitialized(false)
	, m_sdlWindow(nullptr)
	, m_sdlWindowWidth(0)
	, m_sdlWindowHeight(0)
	, m_glContext(nullptr)
	, m_glStateStack(nullptr)
	, m_lineRenderer(nullptr)
	, m_textRenderer(nullptr)
	, m_modelResourceManager(std::unique_ptr<GlModelResourceManager>(new GlModelResourceManager))
	, m_rmlUiRenderer(std::unique_ptr<GlRmlUiRender>(new GlRmlUiRender))
	, m_isRenderingStage(false)
	, m_isRenderingUI(false)
	, m_shaderCache(std::unique_ptr<GlShaderCache>(new GlShaderCache))
{
}

Renderer::~Renderer()
{
	assert(!m_sdlInitialized);
	assert(m_instance == nullptr);
	assert(m_textRenderer == nullptr);
	assert(m_lineRenderer == nullptr);
}

bool Renderer::startup()
{
	EASY_FUNCTION();

	bool success = true;

	MIKAN_LOG_INFO("Renderer::init()") << "Initializing Renderer Context";
	m_instance = this;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) == 0)
	{
		m_sdlInitialized = true;
	}
	else
	{
		MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize SDL: " << SDL_GetError();
		success = false;
	}

	const char* glsl_version= nullptr;
	if (success)
	{
		// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
		glsl_version = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
		glsl_version = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
		glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		char szWindowTitle[128];
		snprintf(szWindowTitle, sizeof(szWindowTitle), "MikanXR v%s", MIKAN_RELEASE_VERSION_STRING);
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		m_sdlWindow = SDL_CreateWindow(szWindowTitle,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			k_window_pixel_width, k_window_pixel_height,
			window_flags);
		m_sdlWindowWidth = k_window_pixel_width;
		m_sdlWindowHeight = k_window_pixel_height;

		if (m_sdlWindow == NULL)
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize window: " << SDL_GetError();
			success = false;
		}
	}

	if (success)
	{
		m_glContext = SDL_GL_CreateContext(m_sdlWindow);
		if (m_glContext != NULL)
		{
			SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
			SDL_GL_SetSwapInterval(1); // Enable vsync
		}
		else
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize window: " << SDL_GetError();
			success = false;
		}
	}

	if (success && !m_shaderCache->startup())
	{
		MIKAN_LOG_ERROR("Renderer::init") << "Failed to initialize shader cache!";
		success = false;
	}

	if (success)
	{
		cursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		cursor_move = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		cursor_pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		cursor_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
		cursor_cross = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
		cursor_text = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		cursor_unavailable = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
	}

	if (success)
	{
		// Initialize GL Extension Wrangler (GLEW)
		GLenum err;
		glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
		err = glewInit();
		if (err != GLEW_OK) 
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize glew openGL backend";
			success = false;
		}
	}

	if (success)
	{
		if (!m_modelResourceManager->startup())
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize model resource manager";
			success = false;
		}
	}

	if (success)
	{
		m_textRenderer = new GlTextRenderer();
		m_lineRenderer= new GlLineRenderer();
		if (!m_lineRenderer->startup())
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize line renderer";
			success = false;
		}
	}

	if (success)
	{
		if (!m_rmlUiRenderer->startup())
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize RmlUi Renderer";
			success = false;
		}
	}

	if (success)
	{
		glClearColor(k_clear_color.r, k_clear_color.g, k_clear_color.b, k_clear_color.a);
		glViewport(0, 0, m_sdlWindowWidth, m_sdlWindowHeight);

		// Create the OpenGL state flag stack
		m_glStateStack = new GlStateStack();

		// Set default state flags at the base of the stack
		m_glStateStack->pushState()
		.enableFlag(eGlStateFlagType::light0)
		.enableFlag(eGlStateFlagType::texture2d)
		.enableFlag(eGlStateFlagType::depthTest)
		.disableFlag(eGlStateFlagType::cullFace)
		// This has to be enabled since the point drawing shader will use gl_PointSize.
		.enableFlag(eGlStateFlagType::programPointSize);

		// Create a fullscreen viewport for the UI (which creates it's own camera)
		m_uiViewport = std::make_shared<GlViewport>();
	}

	return success;
}

void Renderer::shutdown()
{
	m_uiViewport= nullptr;

	if (m_glStateStack != nullptr)
	{
		delete m_glStateStack;
		m_glStateStack= nullptr;
	}

	if (m_rmlUiRenderer != nullptr)
	{
		m_rmlUiRenderer->shutdown();
	}

	if (m_modelResourceManager != nullptr)
	{
		m_modelResourceManager->shutdown();
	}

	if (m_textRenderer != nullptr)
	{
		delete m_textRenderer;
		m_textRenderer= nullptr;
	}

	if (m_lineRenderer != nullptr)
	{
		m_lineRenderer->shutdown();
		delete m_lineRenderer;
		m_lineRenderer = nullptr;
	}

	// Free cursors
	if (cursor_default != nullptr)
	{
		SDL_FreeCursor(cursor_default);
		cursor_default= nullptr;
	}
	if (cursor_move != nullptr)
	{
		SDL_FreeCursor(cursor_move);
		cursor_move = nullptr;
	}
	if (cursor_pointer != nullptr)
	{
		SDL_FreeCursor(cursor_pointer);
		cursor_pointer = nullptr;
	}
	if (cursor_resize != nullptr)
	{
		SDL_FreeCursor(cursor_resize);
		cursor_resize = nullptr;
	}
	if (cursor_cross != nullptr)
	{
		SDL_FreeCursor(cursor_cross);
		cursor_cross = nullptr;
	}
	if (cursor_text != nullptr)
	{
		SDL_FreeCursor(cursor_text);
		cursor_text = nullptr;
	}
	if (cursor_unavailable != nullptr)
	{
		SDL_FreeCursor(cursor_unavailable);
		cursor_unavailable = nullptr;
	}

	if (m_shaderCache != nullptr)
	{
		m_shaderCache->shutdown();
	}

	if (m_glContext != NULL)
	{
		SDL_GL_DeleteContext(m_glContext);
		m_glContext = NULL;
	}

	if (m_sdlWindow != NULL)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow = NULL;
	}

	if (m_sdlInitialized)
	{
		SDL_Quit();
		m_sdlInitialized = false;
	}

	m_instance = NULL;
}

void Renderer::renderBegin()
{
	EASY_FUNCTION();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setSDLMouseCursor(const std::string& cursor_name)
{
	SDL_Cursor* cursor = nullptr;

	if (cursor_name.empty() || cursor_name == "arrow")
		cursor = cursor_default;
	else if (cursor_name == "move")
		cursor = cursor_move;
	else if (cursor_name == "pointer")
		cursor = cursor_pointer;
	else if (cursor_name == "resize")
		cursor = cursor_resize;
	else if (cursor_name == "cross")
		cursor = cursor_cross;
	else if (cursor_name == "text")
		cursor = cursor_text;
	else if (cursor_name == "unavailable")
		cursor = cursor_unavailable;

	if (cursor)
		SDL_SetCursor(cursor);
}

bool Renderer::onSDLEvent(const SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT)
	{
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				m_sdlWindowWidth = event->window.data1;
				m_sdlWindowHeight = event->window.data2;
			}
			break;
		}
	}

	return m_rmlUiRenderer->onSDLEvent(event);
}

void Renderer::renderStageBegin(GlViewportConstPtr targetViewport)
{
	EASY_FUNCTION();

	m_renderingViewport= targetViewport;
	m_renderingViewport->applyViewport();

	m_isRenderingStage = true;
}

void Renderer::renderStageEnd()
{
	EASY_FUNCTION();

	// Render any line segments emitted by the AppStage
	m_lineRenderer->render(this);

	// Render any glyphs emitted by the AppStage
	m_textRenderer->render(this);

	m_renderingViewport= nullptr;
	m_isRenderingStage = false;
}

void Renderer::renderUIBegin()
{
	EASY_FUNCTION();

	m_renderingViewport= m_uiViewport;
	m_renderingViewport->applyViewport();

	m_rmlUiRenderer->beginFrame(this);

	m_isRenderingUI = true;
}

void Renderer::renderUIEnd()
{
	EASY_FUNCTION();	

	m_rmlUiRenderer->endFrame(this);

	// Render any line segments emitted by the AppStage renderUI phase
	m_lineRenderer->render(this);

	// Render any glyphs emitted by the AppStage renderUI phase
	m_textRenderer->render(this);

	m_renderingViewport= nullptr;
	m_isRenderingUI = false;
}

void Renderer::renderEnd()
{
	EASY_FUNCTION();

	SDL_GL_SwapWindow(m_sdlWindow);
}