//-- includes -----
#include "Renderer.h"
#include "Logger.h"
#include "Version.h"

#include "GlRmlUiRenderer.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"

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

#include "opencv2/core.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/core/utils/logger.hpp"

#include "GlCommon.h"
#include "GlCamera.h"
#include "GlTexture.h"
#include "GlTextRenderer.h"
#include "GlLineRenderer.h"
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
static const int k_window_pixel_width = 1280;
static const int k_window_pixel_height = 720;

static const glm::vec4 k_clear_color = glm::vec4(0.45f, 0.45f, 0.5f, 1.f);

static const glm::vec3 k_frustum_color = glm::vec3(0.1f, 0.7f, 0.3f);

//-- statics -----
Renderer* Renderer::m_instance = NULL;

static void setOpencvLoggingLevel(LogSeverityLevel logLevel)
{
	switch (logLevel)
	{
	case LogSeverityLevel::trace:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);
		break;
	case LogSeverityLevel::debug:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_DEBUG);
		break;
	case LogSeverityLevel::info:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_INFO);
		break;
	case LogSeverityLevel::warning:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
		break;
	case LogSeverityLevel::error:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
		break;
	case LogSeverityLevel::fatal:
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
		break;
	}
}

//-- public methods -----
Renderer::Renderer()
	: m_sdlInitialized(false)
	, m_sdlWindow(nullptr)
	, m_sdlWindowWidth(0)
	, m_sdlWindowHeight(0)
	, m_glContext(nullptr)
	, m_lineRenderer(nullptr)
	, m_textRenderer(nullptr)
	, m_modelResourceManager(std::unique_ptr<GlModelResourceManager>(new GlModelResourceManager))
	, m_imguiContext(nullptr)
	, m_imguiOpenGLBackendInitialised(false)
	, m_imguiSDLBackendInitialised(false)
	, m_rmlUiRenderer(std::unique_ptr<GlRmlUiRender>(new GlRmlUiRender))
	, m_isRenderingStage(false)
	, m_isRenderingUI(false)
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

	// Setup ImGui key-bindings context
	if (success)
	{
		// Setup ImGui context
		IMGUI_CHECKVERSION();
		m_imguiContext= ImGui::CreateContext();
		if (m_imguiContext != NULL)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			io.Fonts->AddFontFromFileTTF(getDefaultJapaneseFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesJapanese());
			//TODO: Find these fonts
			//io.Fonts->AddFontFromFileTTF(getDefaultKoreanFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesKorean();
			//io.Fonts->AddFontFromFileTTF(getDefaultChineseFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesChineseFull();
			//io.Fonts->AddFontFromFileTTF(getDefaultCyrillicFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesCyrillic();
			//io.Fonts->AddFontFromFileTTF(getDefaultThaiFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesThai();
			//io.Fonts->AddFontFromFileTTF(getDefaultVietnameseFontPath().c_str(), 16, NULL, io.Fonts->GetGlyphRangesVietnamese();

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();
		}
		else
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to create imgui context";
			success = false;
		}
	}

	// Setup ImGui SDL backend
	if (success)
	{
		// Setup Platform/Renderer backends
		if (ImGui_ImplSDL2_InitForOpenGL(m_sdlWindow, m_glContext))
		{
			m_imguiSDLBackendInitialised= true;
		}
		else
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize imgui SDL backend";
			success = false;
		}
	}

	// Setup ImGui OpenGL backend
	if (success)
	{
		if (ImGui_ImplOpenGL3_Init(glsl_version))
		{
			m_imguiOpenGLBackendInitialised = true;
		}
		else
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize imgui openGL backend";
			success = false;
		}
	}

	// Setup OpenCL
	if (success)
	{
		if (cv::ocl::haveOpenCL())
		{
			// Test for OpenCL availability
			cv::ocl::Device device = cv::ocl::Device::getDefault();

			const char* DeviceType = "Unknown";
			switch (device.type())
			{
			case cv::ocl::Device::TYPE_DEFAULT:
				DeviceType = "DEFAULT";
				break;
			case cv::ocl::Device::TYPE_CPU:
				DeviceType = "CPU";
				break;
			case cv::ocl::Device::TYPE_GPU:
				DeviceType = "GPU";
				break;
			case cv::ocl::Device::TYPE_ACCELERATOR:
				DeviceType = "ACCELERATOR";
				break;
			case cv::ocl::Device::TYPE_DGPU:
				DeviceType = "DGPU";
				break;
			case cv::ocl::Device::TYPE_IGPU:
				DeviceType = "IGPU";
				break;
			case cv::ocl::Device::TYPE_ALL:
				DeviceType = "ALL";
				break;
			}

			MIKAN_LOG_INFO("OpenCL") << "Device Name: " << device.name();
			MIKAN_LOG_INFO("OpenCL") << "Device Type: " << DeviceType;
			MIKAN_LOG_INFO("OpenCL") << "Device Vendor: " << device.vendorName();
			MIKAN_LOG_INFO("OpenCL") << "Device Version: " << device.version();
			MIKAN_LOG_INFO("OpenCL") << "OpenCL Version: " << device.OpenCLVersion();
			MIKAN_LOG_INFO("OpenCL") << "Has Kernel Compiler: " << (device.compilerAvailable() ? "YES" : "NO");
			MIKAN_LOG_INFO("OpenCL") << "Has Kernel Linker: " << (device.linkerAvailable() ? "YES" : "NO");

			// Set the log level in OpenCV
			setOpencvLoggingLevel(LogSeverityLevel::warning);
		}
		else
		{
			MIKAN_LOG_ERROR("Renderer::init") << "Unable to initialize OpenCL";
			success = false;
		}
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

		glEnable(GL_LIGHT0);
		glEnable(GL_TEXTURE_2D);
		//glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		// This has to be enabled since the point drawing shader will use gl_PointSize.
		glEnable(GL_PROGRAM_POINT_SIZE);
		//glDepthFunc(GL_LEQUAL);
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);

		// Create the base camera on the camera stack
		pushCamera();

		m_instance = this;
	}

	return success;
}

void Renderer::shutdown()
{
	while (m_cameraStack.size() > 0)
	{
		popCamera();
	}

	if (m_rmlUiRenderer != nullptr)
	{
		m_rmlUiRenderer->shutdown();
	}

	if (m_modelResourceManager != nullptr)
	{
		m_modelResourceManager->cleanup();
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

	if (m_imguiOpenGLBackendInitialised)
	{
		ImGui_ImplOpenGL3_Shutdown();
		m_imguiOpenGLBackendInitialised= false;
	}

	if (m_imguiSDLBackendInitialised)
	{
		ImGui_ImplSDL2_Shutdown();
		m_imguiSDLBackendInitialised= false;
	}

	if (m_imguiContext != NULL)
	{
		ImGui::DestroyContext(m_imguiContext);
		m_imguiContext= NULL;
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

	m_rmlUiRenderer->onSDLEvent(event);

	return ImGui_ImplSDL2_ProcessEvent(event);
}

void Renderer::renderStageBegin()
{
	EASY_FUNCTION();

	GlCamera *camera= getCurrentCamera();

	if (camera != nullptr)
	{
		glm::mat4 projection = camera->getProjectionMatrix();
		glm::mat4 modelView = camera->getModelViewMatrix();

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(projection));

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(modelView));
	}

	m_isRenderingStage = true;
}

void Renderer::renderStageEnd()
{
	EASY_FUNCTION();

	// Render any line segments emitted by the AppStage
	m_lineRenderer->render();

	// Render any glyphs emitted by the AppStage
	m_textRenderer->render();

	m_isRenderingStage = false;
}

void Renderer::renderUIBegin()
{
	EASY_FUNCTION();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	m_rmlUiRenderer->beginFrame();

	m_isRenderingUI = true;
}

void Renderer::renderUIEnd()
{
	EASY_FUNCTION();

	ImGui::Render();
	glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	m_rmlUiRenderer->endFrame();

	m_isRenderingUI = false;
}

void Renderer::renderEnd()
{
	EASY_FUNCTION();

	SDL_GL_SwapWindow(m_sdlWindow);
}

std::string Renderer::getRmlContextName() const
{
	return m_rmlUiRenderer->GetContext()->GetName();
}

GlCamera* Renderer::getCurrentCamera() const
{
	return m_cameraStack.size() > 0 ? m_cameraStack[m_cameraStack.size() - 1] : nullptr;
}

GlCamera* Renderer::pushCamera()
{
	GlCamera* newCamera= new GlCamera();
	m_cameraStack.push_back(newCamera);

	return newCamera;
}

void Renderer::popCamera()
{
	if (m_cameraStack.size() > 0)
	{
		delete m_cameraStack[m_cameraStack.size() - 1];
		m_cameraStack.pop_back();
	}
}

bool saveTextureToPNG(GlTexture* texture, const char* filename)
{
	int depth = 0;
	switch (texture->getBufferFormat())
	{
	case GL_RGB:
		depth = 24;
		break;
	case GL_RGBA:
		depth = 32;
		break;
	default:
		break;
	}

	if (texture->getTextureWidth() == 0 || texture->getTextureHeight() == 0 || depth < 24)
	{
		return false;
	}

	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int shift = (depth == 24) ? 8 : 0;
	rmask = 0xff000000 >> shift;
	gmask = 0x00ff0000 >> shift;
	bmask = 0x0000ff00 >> shift;
	amask = 0x000000ff >> shift;
#else // little endian, like x86
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = (depth == 24) ? 0 : 0xff000000;
#endif

	const int bytesPerPixel = depth / 8;
	const int pitch = texture->getTextureWidth() * bytesPerPixel;
	uint8_t* buffer = new uint8_t[pitch * texture->getTextureHeight()];

	texture->copyTextureIntoBuffer(buffer);

	SDL_Surface* surface =
		SDL_CreateRGBSurfaceFrom(
			(void*)buffer,
			texture->getTextureWidth(), texture->getTextureHeight(),
			32, pitch,
			rmask, gmask, bmask, amask);

	bool bSuccess = false;
	if (surface != nullptr)
	{
		bSuccess = IMG_SavePNG(surface, filename) == 0;
		SDL_FreeSurface(surface);
	}

	delete[] buffer;

	return bSuccess;
}

