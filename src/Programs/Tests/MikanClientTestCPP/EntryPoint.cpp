//-- includes -----
#include "MikanAPI.h"

#define SDL_MAIN_HANDLED

#include <GL/glew.h>

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

#include "IMkTexture.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "Logger.h"

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

#define MIKAN_CLIENT_ID     "MikanClientTestDX"


class MikanTestApp_GL
{
public:
	MikanTestApp_GL()
		: 
		, m_mikanApi(IMikanAPI::createMikanAPI())
	{
		m_originSpatialAnchorXform = glm::mat4(1.f);

		m_stencilQuad= MikanQuadStencilComponentValues();
		m_stencilQuad.component_id= INVALID_MIKAN_ID;
	}

	virtual ~MikanTestApp_GL()
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


		return success;
	}

	void shutdown()
	{

	}

	void onSDLEvent(SDL_Event& e)
	{
	}

	void update()
	{
	}

	void render()
	{

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

//-- entry point -----
extern "C" int main(int argc, char* argv[])
{
	MikanTestApp_GL app;

	return app.exec(argc, argv);
}