#include "IMkTexture.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "IMkShaderCache.h"
#include "IMkTextureCache.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWindow.h"
#include "IMkState.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "MkMaterialInstance.h"
#include "TestApp.h"
#include "TestGraphicsContext_GL.h"
#include "TestCameraRenderTarget_GL.h"
#include "Logger.h"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// -- Internal Mikan Window adapter -----
class TestMkWindow : public IMkWindow
{
public:
	TestMkWindow(int windowWidth, int windowHeight)
		: IMkWindow()
		, m_mkStateStack(MkStateStackUniquePtr(new MkStateStack(this)))
		, m_shaderCache(createMkShaderCache(this))
		, m_textureCache(createMkTextureCache(this))
		, m_sdlWindow(nullptr)
		, m_glContext(nullptr)
		, m_windowWidth(windowWidth)
		, m_windowHeight(windowHeight)
	{
	}

	virtual ~TestMkWindow() {}

	SDL_Window* getSDLWindow() const
	{
		return m_sdlWindow;
	}

	virtual bool startup() override 
	{
		const char* glsl_version = nullptr;
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
			m_windowWidth, m_windowHeight,
			window_flags);
		if (m_sdlWindow == nullptr)
		{
			MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
			return false;
		}

		m_glContext = SDL_GL_CreateContext(m_sdlWindow);
		if (m_glContext != NULL)
		{
			SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
			SDL_GL_SetSwapInterval(1); // Enable vsync
		}
		else
		{
			MIKAN_LOG_ERROR("startup") << "Unable to initialize window: " << SDL_GetError();
			return false;
		}

		{
			// Initialize GL Extension Wrangler (GLEW)
			GLenum err;
			glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
			err = glewInit();
			if (err != GLEW_OK)
			{
				MIKAN_LOG_ERROR("startup") << "Unable to initialize glew openGL backend";
				return false;
			}
		}

		if (!m_shaderCache->startup())
		{
			MIKAN_LOG_ERROR("startup") << "Failed to initialize shader cache!";
			return false;
		}

		if (!m_textureCache->startup())
		{
			MIKAN_LOG_ERROR("startup") << "Failed to initialize texture cache!";
			return false;
		}

		// Set default state flags at the base of the stack
		{
			IMkState* mkBaseState = m_mkStateStack->pushState("Root Scope");
			assert(mkBaseState->getStackDepth() == 0);

			mkBaseState
				->enableFlag(eMkStateFlagType::texture2d)
				->enableFlag(eMkStateFlagType::depthTest)
				->disableFlag(eMkStateFlagType::cullFace);

			static const glm::vec4 k_clear_color = glm::vec4(0.45f, 0.45f, 0.5f, 1.f);
			mkStateSetClearColor(mkBaseState, k_clear_color);
			mkStateSetViewport(mkBaseState, 0, 0, m_windowWidth, m_windowHeight);
		}

		return true;
	}

	virtual void update(float deltaSeconds) override {}

	virtual void render() override 
	{
		SDL_GL_SwapWindow(m_sdlWindow);
	}

	virtual void shutdown() override 
	{
		m_textureCache->shutdown();
		m_shaderCache->shutdown();

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
	}

	virtual float getWidth() const override { return (float)m_windowWidth; }
	virtual float getHeight() const override { return (float)m_windowHeight; }
	virtual float getAspectRatio() const override { return (float)m_windowWidth / (float)m_windowHeight; }
	virtual bool getIsRenderingStage() const override { return false; }
	virtual bool getIsRenderingUI() const override { return false; }
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }
	virtual MkStateStack& getMkStateStack() override { return *m_mkStateStack.get(); }
	virtual class IMkLineRenderer* getLineRenderer() override { return nullptr; }
	virtual class IMkTextRenderer* getTextRenderer() override { return nullptr; }
	virtual IMkShaderCache* getShaderCache() override { return m_shaderCache.get(); }
	virtual IMkTextureCache* getTextureCache() override { return m_textureCache.get(); }

private:
	MkStateStackUniquePtr m_mkStateStack;
	IMkShaderCachePtr m_shaderCache;
	IMkTextureCachePtr m_textureCache;

	SDL_Window* m_sdlWindow;
	void* m_glContext;
	int m_windowWidth;
	int m_windowHeight;
};

// -- TestGraphicsContext_GL Implementation -----
TestGraphicsContext_GL::TestGraphicsContext_GL(TestApp* ownerApp)
	: TestGraphicsContext(ownerApp)
{}

MkStateStack& TestGraphicsContext_GL::getMkStateStack()
{
	return m_mkWindow->getMkStateStack();
}

SDL_Window* TestGraphicsContext_GL::getSDLWindow() const
{
	if (m_mkWindow)
	{
		return std::static_pointer_cast<TestMkWindow>(m_mkWindow)->getSDLWindow();
	}

	return nullptr;
}

TestCameraRenderTargetPtr TestGraphicsContext_GL::allocateCameraRenderTarget(int cameraId)
{
	return std::make_shared<TestCameraRenderTarget_GL>(shared_from_this(), cameraId);
}

bool TestGraphicsContext_GL::create(int windowWidth, int windowHeight)
{
	// Initialize the internal MkWindow
	m_mkWindow = std::make_shared<TestMkWindow>(windowWidth, windowHeight);
	if (!m_mkWindow->startup())
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize TestMkWindow";
		return false;
	}

	// Create a fullscreen quad mesh for rendering camera targets to the main framebuffer
	m_viewportQuadMesh = createFullscreenQuadMesh(m_mkWindow.get(), false);

	return true;
}

bool TestGraphicsContext_GL::initializeCubeGeometry()
{
	auto stencilMaterial = m_mkWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PC_UNLIT_COLOR);
	assert(stencilMaterial);

	struct BoxVertex
	{
		glm::vec3 inPosition;
		glm::vec4 inColor;
	};
	size_t vertexSize = sizeof(BoxVertex);

	// Create triangulated box mesh to draw the box stencils
	{
		static BoxVertex x_vertices[] = {
			// positions                     colors
			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)}, // Front
			{glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)},

			{glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)}, // Back
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)},

			{glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)}, // Top
			{glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)},

			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)}, // Bottom
			{glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)},

			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)}, // Left
			{glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
			{glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},

			{glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)}, // Right
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
			{glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)},
		};

		static uint16_t x_indices[] = {
			 0,  1,  2,  3,  4,  5, // Front
			 6,  7,  8,  9, 10, 11, // Back
			12, 13, 14, 15, 16, 17, // Top
			18, 19, 20, 21, 22, 23, // Bottom
			24, 25, 26, 27, 28, 29, // Left
			30, 31, 32, 33, 34, 35  // Right
		};

		m_boxMesh =
			createMkTriangulatedMesh(
				m_mkWindow.get(),
				"box_mesh",
				(const uint8_t*)x_vertices,
				vertexSize,
				6*6, // 6 faces * 6 vertices per face
				(const uint8_t*)x_indices,
				sizeof(uint16_t), // 2 bytes per index
				12, // 12 tris in a cube (6 faces * 2 tris/face)
				false); // mesh doesn't own box vertex data
		if (!m_boxMesh->setMaterial(stencilMaterial) ||
			!m_boxMesh->createResources())
		{
			MIKAN_LOG_ERROR("initializeCubeGeometry") << "Failed to create box mesh";
			return false;
		}
	}

	return true;
}

void TestGraphicsContext_GL::recreateMainRenderTarget()
{

}

void TestGraphicsContext_GL::renderMainTarget() const
{
	TestCameraRenderTargetPtr renderTarget= getCameraRenderTarget(m_lastRenderedCameraId);

	if (renderTarget)
	{
		auto glRenderTarget = std::static_pointer_cast<TestCameraRenderTarget_GL>(renderTarget);
		IMkTextureConstPtr colorTexture = glRenderTarget->getColorTexture();

		if (colorTexture)
		{
			MkMaterialInstancePtr materialInstance = m_viewportQuadMesh->getMaterialInstance();
			MkMaterialConstPtr material = materialInstance->getMaterial();

			if (auto materialBinding = material->bindMaterial())
			{
				// Bind the color texture
				materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, colorTexture);

				// Draw the color texture
				if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
				{
					MkScopedState scopedState = 
						m_mkWindow->getMkStateStack().createScopedState("MainTargetComponentRender");
					scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

					m_viewportQuadMesh->drawElements();
				}
			}
		}
	}

	// Draw the window contents to the screen
	m_mkWindow->render();
}

bool TestGraphicsContext_GL::renderToCameraTarget(
	TestCameraRenderTarget* cameraRenderTarget)
{
	auto* glRenderTarget = static_cast<TestCameraRenderTarget_GL*>(cameraRenderTarget);

	// Render the scene
	MkMaterialInstancePtr materialInstance = m_boxMesh->getMaterialInstance();
	MkMaterialConstPtr material = materialInstance->getMaterial();

	if (auto materialBinding = material->bindMaterial())
	{
		// Bind the color texture
		const glm::mat4 vpMatrix = glRenderTarget->getViewProjectionMatrix();

		const float time = m_ownerApp->getTimeSeconds();
		const MikanVector3f& cubeOffset = m_ownerApp->getCubeOffset();
		const glm::vec3 cameraPosition = glRenderTarget->getCameraPosition();
		const glm::vec3 cameraForward = glRenderTarget->getCameraForward();
		const glm::vec3 cameraUp = glRenderTarget->getCameraUp();
		const glm::vec3 cameraRight = glRenderTarget->getCameraRight();

		glm::vec3 cubePosition =
			cameraPosition +
			cameraForward * cubeOffset.z +
			cameraUp * cubeOffset.y +
			cameraRight * cubeOffset.x;

		glm::mat4 cubeXform =
			glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)) *
			glm::rotate(glm::mat4(1.0f), time, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), time * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), time * 0.7f, glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::translate(glm::mat4(1.0f), cubePosition);

		materialInstance->setMat4BySemantic(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * cubeXform);

		// Draw the color texture
		if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
		{
			MkScopedState scopedState =
				m_mkWindow->getMkStateStack().createScopedState("MainTargetComponentRender");
			scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

			m_viewportQuadMesh->drawElements();
		}
	}

	// Remember the last rendered camera ID
	// We will render this camera in renderMainTarget
	m_lastRenderedCameraId = cameraRenderTarget->getCameraId();

	return true;
}

void TestGraphicsContext_GL::dispose()
{
	if (m_boxMesh)
	{
		m_boxMesh->deleteResources();
		m_boxMesh = nullptr;
	}

	if (m_viewportQuadMesh)
	{
		m_viewportQuadMesh->deleteResources();
		m_viewportQuadMesh = nullptr;
	}

	if (m_mkWindow)
	{
		m_mkWindow->shutdown();
		m_mkWindow = nullptr;
	}
}