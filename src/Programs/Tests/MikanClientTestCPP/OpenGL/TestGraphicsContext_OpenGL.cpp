#include "TestGraphicsContext_OpenGL.h"
#include "IMkTexture.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "Logger.h"

#include <GL/glew.h>

#define SDL_MAIN_HANDLED
#if defined(_WIN32)
	#include <SDL.h>
	#include <SDL_events.h>
	#include <SDL_syswm.h>
	#if defined(IMGUI_IMPL_OPENGL_ES2)
		#include <SDL_opengles2.h>
		#include <SDL_opengles2_gl2.h>
	#else
		#include <SDL_opengl.h>
		#include <SDL_opengl_glext.h>
	#endif
#else
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_events.h>
	#include <SDL2/SDL_syswm.h>
	#if defined(IMGUI_IMPL_OPENGL_ES2)
		#include <SDL2/SDL_opengles2.h>
		#include <SDL2/SDL_opengles2_gl2.h>
	#else
		#include <SDL2/SDL_opengl.h>
		#include <SDL2/SDL_opengl_glext.h>
	#endif
#endif

#include <memory>

TestGraphicsContext_OpenGL::TestGraphicsContext_OpenGL()
	: ITestGraphicsContext()
	, m_mkStateStack(MkStateStackUniquePtr(new MkStateStack(nullptr)))
{}

MkStateStack& TestGraphicsContext_OpenGL::getMkStateStack()
{
	return *m_mkStateStack.get();
}

TestCameraRenderTargetPtr TestGraphicsContext_OpenGL::allocateCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId)
{
	return std::make_shared<MikanCameraRenderTarget_GL>(mikanAPI, cameraId);
}

bool TestGraphicsContext_OpenGL::create(int windowWidth, int windowHeight)
{
	bool success = true;

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
			windowWidth, windowHeight,
			window_flags);
		m_windowWidth = windowWidth;
		m_windowHeight = windowHeight;

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
		m_shader = compileShader(getShaderCode());
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
		success = createFrameBuffer(m_windowWidth, m_windowHeight);
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
	}

	return success;
}

void TestGraphicsContext_OpenGL::disposeWindow()
{
	freeFrameBuffer();
	freeVertexBuffers();

	if (m_cubeTexture != nullptr)
	{
		m_cubeTexture->disposeTexture();
		m_cubeTexture = nullptr;
	}

	if (m_floorTexture != nullptr)
	{
		m_floorTexture->disposeTexture();
		m_floorTexture = nullptr;
	}

	if (m_shader != nullptr)
	{
		m_shader = nullptr;
	}

	if (m_screenShader != nullptr)
	{
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
}

#if 0
const float k_default_camera_vfov = 35.f;
const float k_default_camera_z_near = 0.1f;
const float k_default_camera_z_far = 5000.f;

static const glm::vec4 k_background_color_key = glm::vec4(0.f, 0.f, 0.0f, 0.f);

#define k_real_pi 3.14159265f
#define degrees_to_radians(x) (((x) * k_real_pi) / 180.f)

#define SCENE_SHADER_MVP_UNIFORM		"mvpMatrix"
#define SCENE_SHADER_DIFFUSE_UNIFORM	"diffuse"

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

	const IMkShaderCodePtr getShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;
		
		if (x_shaderCode == nullptr)
		{
			x_shaderCode= createIMkShaderCode(
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
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform(SCENE_SHADER_MVP_UNIFORM, eUniformSemantic::modelViewProjectionMatrix);
			x_shaderCode->addUniform(SCENE_SHADER_DIFFUSE_UNIFORM, eUniformSemantic::rgbTexture);
		}

		return x_shaderCode;
	}

	const IMkShaderCodePtr getScreenShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode= createIMkShaderCode(
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
			)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("screenTexture", eUniformSemantic::rgbTexture);
		}

		return x_shaderCode;
	}

	IMkShaderPtr compileShader(IMkShaderCodeConstPtr shaderCode)
	{
		IMkShaderPtr shader = createIMkShader(shaderCode);

		if (!shader->compileProgram())
		{			
			shader= nullptr;
		}

		return shader;
	}

	IMkTexturePtr loadTexture(char const* path)
	{
		IMkTexturePtr texture= CreateMkTexture();

		texture->setImagePath(path);
		if (!texture->reloadTextureFromImagePath())
		{
			MIKAN_LOG_ERROR("loadTexture") << "Texture failed to load at path: " << path;
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
		m_textureColorbuffer = CreateMkTexture();
		m_textureColorbuffer->setSize(width, height);
		m_textureColorbuffer->setTextureFormat(GL_RGBA);
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
	float m_zNear, m_zFar;

	IMkShaderPtr m_shader= nullptr;
	IMkShaderPtr m_screenShader= nullptr;

	unsigned int m_cubeVAO= 0, m_cubeVBO= 0;
	unsigned int m_planeVAO= 0, m_planeVBO= 0;
	unsigned int m_quadVAO= 0, m_quadVBO= 0;

	IMkTexturePtr m_cubeTexture= nullptr;
	IMkTexturePtr m_floorTexture= nullptr;

	IMkTexturePtr m_textureColorbuffer= nullptr;
	unsigned int m_framebuffer= 0;
	unsigned int m_rbo= 0;

	IMikanAPIPtr m_mikanApi;
	int64_t m_lastReceivedVideoSourceFrame= 0;
	glm::mat4 m_originSpatialAnchorXform;
	MikanQuadStencilComponentValues m_stencilQuad;
	glm::mat4 m_cameraOffsetXform= glm::mat4(1.f);	
	float m_mikanReconnectTimeout= 0.f; // seconds

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested = false;

	// Current FPS rate
	uint32_t m_lastFrameTimestamp = 0;
	float m_fps = 0.f;
};
#endif