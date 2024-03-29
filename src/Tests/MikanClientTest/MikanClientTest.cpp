//-- includes -----
#include "MikanAPI.h"
#include "MikanEventTypes.h"
#include "MikanMathTypes.h"
#include "MikanStencilTypes.h"
#include "MikanVideoSourceTypes.h"

#define SDL_MAIN_HANDLED

#if defined(_WIN32)
	#include <SDL.h>
	#include <SDL_events.h>
	#include <SDL_syswm.h>
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_events.h>
	#include <SDL2/SDL_syswm.h>
#endif

#include "GlCommon.h"
#include "GlTexture.h"
#include "GlProgram.h"
#include "Logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <memory>

#include "stdio.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)  // ignore strncpy warning
#endif

static const int k_window_pixel_width = 1280;
static const int k_window_pixel_height = 600;

const float k_default_camera_vfov = 35.f;
const float k_default_camera_z_near = 0.1f;
const float k_default_camera_z_far = 5000.f;

static const glm::vec4 k_background_color_key = glm::vec4(0.f, 0.f, 0.0f, 0.f);

#define k_real_pi 3.14159265f
#define degrees_to_radians(x) (((x) * k_real_pi) / 180.f)

#define SCENE_SHADER_MVP_UNIFORM		"mvpMatrix"
#define SCENE_SHADER_DIFFUSE_UNIFORM	"diffuse"

glm::mat4 MikanMatrix4f_to_glm_mat4(const MikanMatrix4f& xform)
{
	auto m = reinterpret_cast<const float(*)[4][4]>(&xform);

	glm::mat4 mat = {
		{(*m)[0][0], (*m)[0][1], (*m)[0][2], (*m)[0][3]}, // columns 0
		{(*m)[1][0], (*m)[1][1], (*m)[1][2], (*m)[1][3]}, // columns 1
		{(*m)[2][0], (*m)[2][1], (*m)[2][2], (*m)[2][3]}, // columns 2
		{(*m)[3][0], (*m)[3][1], (*m)[3][2], (*m)[3][3]}, // columns 3
	};

	return mat;
}

glm::vec3 MikanVector3f_to_glm_vec3(const MikanVector3f& in)
{
	return glm::vec3(in.x, in.y, in.z);
}

MikanVector3f glm_vec3_to_MikanVector3f(const glm::vec3& in)
{
	return {in.x, in.y, in.z};
}

class MikanTestApp
{
public:
	MikanTestApp()
		: m_mikanApi(IMikanAPI::createMikanAPI())
	{
		m_originSpatialAnchorXform = glm::mat4(1.f);

		memset(&m_stencilQuad, 0, sizeof(MikanStencilQuad));
		m_stencilQuad.stencil_id= INVALID_MIKAN_ID;
	}

	virtual ~MikanTestApp()
	{
		shutdown();
	}

	int exec(int argc, char** argv)
	{
		int result = 0;

		if (startup(argc, argv))
		{
			SDL_Event e;

			while (!m_bShutdownRequested)
			{
				if (SDL_PollEvent(&e))
				{
					if (e.type == SDL_QUIT ||
						(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
					{
						MIKAN_LOG_INFO("exec") << "QUIT message received";
						break;
					}
					else
					{
						onSDLEvent(e);
					}
				}

				update();
			}
		}
		else
		{
			MIKAN_LOG_ERROR("exec") << "Failed to initialize application!";
			result = -1;
		}

		shutdown();

		return result;
	}

	inline void requestShutdown()
	{
		m_bShutdownRequested = true;
	}

protected:
	bool startup(int argc, char** argv)
	{
		bool success = true;

		LoggerSettings settings = {};
		settings.min_log_level = LogSeverityLevel::info;
		settings.enable_console = true;

		log_init(settings);

		if (m_mikanApi->init(MikanLogLevel_Info, onMikanLog) == MikanResult_Success)
		{
			MikanClientInfo ClientInfo = {};
			ClientInfo.supportsRBG24 = true;
			ClientInfo.engineName = "MikanXR Test";
			ClientInfo.engineVersion = "1.0";
			ClientInfo.applicationName = "MikanXR Test";
			ClientInfo.applicationVersion = "1.0";
			ClientInfo.graphicsAPI = MikanClientGraphicsApi_OpenGL;

			m_mikanApi->setClientInfo(ClientInfo);

			m_mikanInitialized= true;
		}
		else
		{
			MIKAN_LOG_ERROR("startup") << "Failed to initialize Mikan Client API";
			success= false;
		}

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) == 0)
		{
			m_sdlInitialized = true;
		}
		else
		{
			MIKAN_LOG_ERROR("startup") << "Unable to initialize SDL: " << SDL_GetError();
			success = false;
		}

		const char* glsl_version = nullptr;
		if (success)
		{
			// Decide GL+GLSL versions
#if defined(__APPLE__)
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

			SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
			m_sdlWindow = SDL_CreateWindow("Mikan Client Test",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				k_window_pixel_width, k_window_pixel_height,
				window_flags);
			m_sdlWindowWidth = k_window_pixel_width;
			m_sdlWindowHeight = k_window_pixel_height;

			if (m_sdlWindow == NULL)
			{
				MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
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
				MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
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
				MIKAN_LOG_ERROR("startup") << "Unable to initialize glew openGL backend";
				success = false;
			}
		}

		if (success)
		{
			m_shader= compileShader(getShaderCode());
			m_screenShader = compileShader(getScreenShaderCode());

			if (m_shader == nullptr || m_screenShader == nullptr)
			{
				MIKAN_LOG_ERROR("startup") << "Failed to compile shaders";
				success = false;
			}
		}

		if (success)
		{
			m_cubeTexture = loadTexture("resources/textures/container.jpg");
			m_floorTexture = loadTexture("resources/textures/space.png");

			if (m_cubeTexture == nullptr || m_floorTexture == nullptr)
			{
				MIKAN_LOG_ERROR("startup") << "Failed to load textures";
				success = false;
			}
		}

		if (success)
		{
			success= createFrameBuffer(m_sdlWindowWidth, m_sdlWindowHeight);
		}

		if (success)
		{
			createVertexBuffers();

			glClearColor(k_background_color_key.r, k_background_color_key.g, k_background_color_key.b, 0.f);
			glViewport(0, 0, m_sdlWindowWidth, m_sdlWindowHeight);

			glEnable(GL_LIGHT0);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			m_viewMatrix = 
				glm::lookAt(
					glm::vec3(0.f, 0.f, 3.f),
					glm::vec3(0.f, 0.f, 0.f),	// Look at tracking origin
					glm::vec3(0.f, 1.f, 0.f));  // +Y is up.
			m_projectionMatrix =
				glm::perspective(
					degrees_to_radians(k_default_camera_vfov),
					(float)m_sdlWindowWidth / (float)m_sdlWindowHeight,
					k_default_camera_z_near,
					k_default_camera_z_far);
		}

		if (success)
		{
			m_lastFrameTimestamp = SDL_GetTicks();
		}

		return success;
	}

	void shutdown()
	{
		freeFrameBuffer();
		freeVertexBuffers();

		if (m_cubeTexture != nullptr)
		{
			m_cubeTexture->disposeTexture();
			delete m_cubeTexture;
			m_cubeTexture = nullptr;
		}

		if (m_floorTexture != nullptr)
		{
			m_floorTexture->disposeTexture();
			delete m_floorTexture;
			m_floorTexture= nullptr;
		}

		if (m_shader != nullptr)
		{
			delete m_shader;
			m_shader= nullptr;
		}

		if (m_screenShader != nullptr)
		{
			delete m_screenShader;
			m_screenShader = nullptr;
		}

		if (m_glContext != nullptr)
		{
			SDL_GL_DeleteContext(m_glContext);
			m_glContext = nullptr;
		}

		if (m_sdlWindow != nullptr)
		{
			SDL_DestroyWindow(m_sdlWindow);
			m_sdlWindow = nullptr;
		}

		if (m_sdlInitialized)
		{
			SDL_Quit();
			m_sdlInitialized = false;
		}

		if (m_mikanInitialized)
		{
			m_mikanApi->shutdown();
			m_mikanInitialized= false;
		}

		log_dispose();
	}

	static void onMikanLog(int log_level, const char* log_message)
	{
		switch (log_level)
		{
			case MikanLogLevel_Debug:
				MIKAN_LOG_DEBUG("onMikanLog") << log_message;
				break;
			case MikanLogLevel_Info:
				MIKAN_LOG_INFO("onMikanLog") << log_message;
				break;
			case MikanLogLevel_Warning:
				MIKAN_LOG_WARNING("onMikanLog") << log_message;
				break;
			case MikanLogLevel_Error:
				MIKAN_LOG_ERROR("onMikanLog") << log_message;
				break;
			case MikanLogLevel_Fatal:
				MIKAN_LOG_FATAL("onMikanLog") << log_message;
				break;
			default:
				MIKAN_LOG_INFO("onMikanLog") << log_message;
				break;
		}
	}

	void onSDLEvent(SDL_Event& e)
	{
	}

	void update()
	{
		// Update the frame rate
		const uint32_t now = SDL_GetTicks();
		const float deltaSeconds = (float)(now - m_lastFrameTimestamp) / 1000.f;
		m_fps = deltaSeconds > 0.f ? (1.0f / deltaSeconds) : 0.f;
		m_lastFrameTimestamp = now;

		if (m_mikanApi->getIsConnected())
		{
			MikanEventPtr event;
			while (m_mikanApi->fetchNextEvent(event) == MikanResult_Success)
			{
				if (event->eventType == MikanConnectedEvent::k_typeName)
				{
					reallocateRenderBuffers();
					updateCameraProjectionMatrix();
				}
				else if (event->eventType == MikanVideoSourceOpenedEvent::k_typeName)
				{
					reallocateRenderBuffers();
					updateCameraProjectionMatrix();
				}
				else if (event->eventType == MikanVideoSourceNewFrameEvent::k_typeName)
				{
					auto newFrameEvent = std::static_pointer_cast<MikanVideoSourceNewFrameEvent>(event);
					processNewVideoSourceFrame(*newFrameEvent.get());
				}
				else if (event->eventType == MikanVideoSourceModeChangedEvent::k_typeName ||
						 event->eventType == MikanVideoSourceIntrinsicsChangedEvent::k_typeName)
				{
					reallocateRenderBuffers();
					updateCameraProjectionMatrix();
				}
			}
		}
		else
		{
			if (m_mikanReconnectTimeout <= 0.f)
			{
				if (m_mikanApi->connect() != MikanResult_Success)
				{
					// timeout between reconnect attempts
					m_mikanReconnectTimeout= 1.0f; 
				}
			}
			else
			{
				m_mikanReconnectTimeout-= deltaSeconds;
			}

			// Just render the scene using the last applied camera pose
			render();
		}
	}

	void processNewVideoSourceFrame(const MikanVideoSourceNewFrameEvent& newFrameEvent)
	{
		if (newFrameEvent.frame == m_lastReceivedVideoSourceFrame)
			return;

		// Apply the camera pose received
		setCameraPose(
			MikanVector3f_to_glm_vec3(newFrameEvent.cameraForward),
			MikanVector3f_to_glm_vec3(newFrameEvent.cameraUp),
			MikanVector3f_to_glm_vec3(newFrameEvent.cameraPosition));

		// Render out a new frame
		render();

		// Publish the new video frame back to Mikan
		GLuint textureId = m_textureColorbuffer->getGlTextureId();
		MikanClientFrameRendered frameRendered = {newFrameEvent.frame};
		m_mikanApi->publishRenderTargetTexture(&textureId, frameRendered);

		// Remember the frame index of the last frame we published
		m_lastReceivedVideoSourceFrame= newFrameEvent.frame;
	}

	void setCameraPose(
		const glm::vec3& cameraForward,
		const glm::vec3& cameraUp, 
		const glm::vec3& cameraPosition)
	{
		m_viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
	}

	void reallocateRenderBuffers()
	{
		// Free the old frame buffer, if any
		freeFrameBuffer();

		// Tell the server to free the old render target buffers
		m_mikanApi->freeRenderTargetBuffers().wait();

		// Fetch the current video source resolution
		auto future= m_mikanApi->getVideoSourceAPI()->getVideoSourceMode();
		auto response= future.get();
		if (response->resultCode == MikanResult_Success)
		{
			auto mode = std::static_pointer_cast<MikanVideoSourceMode>(response);

			MikanRenderTargetDescriptor desc;
			memset(&desc, 0, sizeof(MikanRenderTargetDescriptor));
			desc.width = mode->resolution_x;
			desc.height = mode->resolution_y;
			desc.color_buffer_type = MikanColorBuffer_RGBA32;
			desc.depth_buffer_type = MikanDepthBuffer_NODEPTH;
			desc.graphicsAPI = MikanClientGraphicsApi_OpenGL;

			// Tell the server to allocate new render target buffers
			m_mikanApi->allocateRenderTargetBuffers(desc).wait();

			// Create a new frame buffer to render to
			createFrameBuffer(mode->resolution_x, mode->resolution_y);
		}
	}

	void updateCameraProjectionMatrix()
	{
		auto future= m_mikanApi->getVideoSourceAPI()->getVideoSourceIntrinsics();
		auto response= future.get();

		if (response->resultCode == MikanResult_Success)
		{
			auto videoSourceIntrinsics= std::static_pointer_cast<MikanVideoSourceIntrinsics>(response);

			const MikanMonoIntrinsics& monoIntrinsics= videoSourceIntrinsics->intrinsics.mono;
			const float videoSourcePixelWidth = monoIntrinsics.pixel_width;
			const float videoSourcePixelHeight = monoIntrinsics.pixel_height;

			m_projectionMatrix =
				glm::perspective(
					(float)degrees_to_radians(monoIntrinsics.vfov),
					videoSourcePixelWidth / videoSourcePixelHeight,
					(float)monoIntrinsics.znear,
					(float)monoIntrinsics.zfar);
		}
	}

	void render()
	{
		// Render the scene
		{
			const glm::mat4 vpMatrix = m_projectionMatrix * m_viewMatrix;

			// Cache the last viewport dimensions
			GLint last_viewport[4];
			glGetIntegerv(GL_VIEWPORT, last_viewport);

			// Change the viewport to match the frame buffer texture
			glViewport(0, 0, m_textureColorbuffer->getTextureWidth(), m_textureColorbuffer->getTextureHeight());


			// bind to framebuffer and draw scene as we normally would to color texture 
			glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

			// make sure we clear the framebuffer's content
			glClearColor(k_background_color_key.r, k_background_color_key.g, k_background_color_key.b, 0.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_shader->bindProgram();

			// Draw a small cube
			{
				m_cubeTexture->bindTexture();

				const glm::mat4 boxXform = m_originSpatialAnchorXform;
				const glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.1f, 0.1f, 0.1f));
				m_shader->setMatrix4x4Uniform(SCENE_SHADER_MVP_UNIFORM, vpMatrix * boxXform * scale);

				glBindVertexArray(m_cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);

				m_cubeTexture->clearTexture();
			}

			// Draw a large skybox
			{
				m_floorTexture->bindTexture();

				const glm::mat4 boxXform = m_originSpatialAnchorXform;
				const glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(10.0f, 10.0f, 10.0f));
				m_shader->setMatrix4x4Uniform(SCENE_SHADER_MVP_UNIFORM, vpMatrix * boxXform * scale);

				glBindVertexArray(m_cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);

				m_floorTexture->clearTexture();
			}

			m_shader->unbindProgram();

			// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

			// Restore the viewport
			glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		}

		// Render the scene to the window
		{
			// clear all relevant buffers
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
			glClear(GL_COLOR_BUFFER_BIT);

			m_screenShader->bindProgram();
			m_textureColorbuffer->bindTexture(); // use the color attachment texture as the texture of the quad plane

			glBindVertexArray(m_quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Capture the back buffer and write it to Mikan
			//if (m_renderTargetMemory.color_buffer != nullptr)
			//{
			//	glGetTexImage(GL_TEXTURE_2D,
			//		0,
			//		GL_RGB,
			//		GL_UNSIGNED_BYTE,
			//		m_renderTargetMemory.color_buffer);
			//}

			m_textureColorbuffer->clearTexture();
			m_screenShader->unbindProgram();
		}

		SDL_GL_SwapWindow(m_sdlWindow);
	}

	const GlProgramCode& getShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			"Scene Shader Code",
			// vertex shader
			R""""(
			#version 330 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec2 aTexCoords;

			out vec2 TexCoords;

			uniform mat4 mvpMatrix;

			void main()
			{
				TexCoords = aTexCoords;    
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D diffuse;

			void main()
			{    
				FragColor = texture(diffuse, TexCoords);
			}
			)"""")
			.addVertexAttributes("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position)
			.addVertexAttributes("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord)
			.addUniform(SCENE_SHADER_MVP_UNIFORM, eUniformSemantic::modelViewProjectionMatrix)
			.addUniform(SCENE_SHADER_DIFFUSE_UNIFORM, eUniformSemantic::rgbTexture);

		return x_shaderCode;
	}

	const GlProgramCode& getScreenShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			"Screen Shader Code",
			// vertex shader
			R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
			}  
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D screenTexture;

			void main()
			{
				vec3 col = texture(screenTexture, TexCoords).rgb;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
			.addVertexAttributes("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position)
			.addVertexAttributes("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord)
			.addUniform("screenTexture", eUniformSemantic::rgbTexture);

		return x_shaderCode;
	}

	GlProgram* compileShader(const GlProgramCode& shaderCode)
	{
		GlProgram* shader = new GlProgram(shaderCode);

		if (!shader->compileProgram())
		{			
			delete shader;
			shader= nullptr;
		}

		return shader;
	}

	GlTexture* loadTexture(char const* path)
	{
		GlTexture* texture= new GlTexture();

		texture->setImagePath(path);
		if (!texture->reloadTextureFromImagePath())
		{
			MIKAN_LOG_ERROR("loadTexture") << "Texture failed to load at path: " << path;
			delete texture;
			texture= nullptr;
		}

		return texture;
	}

	void createVertexBuffers()
	{
		float cubeVertices[] = {
			// positions          // texture Coords
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};

		float planeVertices[] = {
			// positions          // texture Coords 
			 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
			-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
			-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

			 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
			-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
			 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
		};

		float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		// cube VAO
		glGenVertexArrays(1, &m_cubeVAO);
		glGenBuffers(1, &m_cubeVBO);
		glBindVertexArray(m_cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		// plane VAO
		glGenVertexArrays(1, &m_planeVAO);
		glGenBuffers(1, &m_planeVBO);
		glBindVertexArray(m_planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		// screen quad VAO
		glGenVertexArrays(1, &m_quadVAO);
		glGenBuffers(1, &m_quadVBO);
		glBindVertexArray(m_quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

	void freeVertexBuffers()
	{
		if (m_cubeVAO != 0)
		{
			glDeleteVertexArrays(1, &m_cubeVAO);
			m_cubeVAO = 0;
		}
		if (m_planeVAO != 0)
		{
			glDeleteVertexArrays(1, &m_planeVAO);
			m_planeVAO = 0;
		}
		if (m_quadVAO != 0)
		{
			glDeleteVertexArrays(1, &m_quadVAO);
			m_quadVAO = 0;
		}

		if (m_cubeVBO != 0)
		{
			glDeleteBuffers(1, &m_cubeVBO);
			m_cubeVBO = 0;
		}
		if (m_planeVBO != 0)
		{
			glDeleteBuffers(1, &m_planeVBO);
			m_planeVBO = 0;
		}
		if (m_quadVBO != 0)
		{
			glDeleteBuffers(1, &m_quadVBO);
			m_quadVBO= 0;
		}
	}

	bool createFrameBuffer(uint16_t width, uint16_t height)
	{
		bool bSuccess= true;

		
		glGenFramebuffers(1, &m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		
		// create a color attachment texture
		m_textureColorbuffer =
			(new GlTexture())
			->setSize(width, height)
			->setTextureFormat(GL_RGBA);
		m_textureColorbuffer->createTexture();
		m_textureColorbuffer->bindTexture();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorbuffer->getGlTextureId(), 0);
		
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo); // now actually attach it
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		GLenum result= glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE)
		{
			MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
			bSuccess= false;
		}

		return bSuccess;
	}

	void freeFrameBuffer()
	{
		if (m_rbo != 0)
		{
			glDeleteRenderbuffers(1, &m_rbo);
			m_rbo= 0;
		}

		if (m_textureColorbuffer != nullptr)
		{
			m_textureColorbuffer->disposeTexture();
			delete m_textureColorbuffer;
			m_textureColorbuffer= nullptr;
		}

		if (m_framebuffer != 0)
		{
			glDeleteFramebuffers(1, &m_framebuffer);
			m_framebuffer= 0;
		}
	}

private:
	bool m_mikanInitialized= false;
	bool m_sdlInitialized= false;

	SDL_Window* m_sdlWindow= nullptr;
	int m_sdlWindowWidth= 0, m_sdlWindowHeight= 0;

	void* m_glContext= 0;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewMatrix;

	GlProgram* m_shader= nullptr;
	GlProgram* m_screenShader= nullptr;

	unsigned int m_cubeVAO= 0, m_cubeVBO= 0;
	unsigned int m_planeVAO= 0, m_planeVBO= 0;
	unsigned int m_quadVAO= 0, m_quadVBO= 0;

	GlTexture* m_cubeTexture= nullptr;
	GlTexture* m_floorTexture= nullptr;

	GlTexture* m_textureColorbuffer= nullptr;
	unsigned int m_framebuffer= 0;
	unsigned int m_rbo= 0;

	IMikanAPIPtr m_mikanApi;
	uint64_t m_lastReceivedVideoSourceFrame= 0;
	glm::mat4 m_originSpatialAnchorXform;
	MikanStencilQuad m_stencilQuad;
	glm::mat4 m_cameraOffsetXform= glm::mat4(1.f);	
	float m_mikanReconnectTimeout= 0.f; // seconds

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested = false;

	// Current FPS rate
	uint32_t m_lastFrameTimestamp = 0;
	float m_fps = 0.f;
};

//-- entry point -----
extern "C" int main(int argc, char* argv[])
{
	MikanTestApp app;

	return app.exec(argc, argv);
}