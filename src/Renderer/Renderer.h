#pragma once

//-- includes -----
#include "RendererFwd.h"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

//-- typedefs -----
typedef union SDL_Event SDL_Event;

//-- definitions -----
class Renderer
{
public:
	Renderer();
	~Renderer();

	bool startup();
	void shutdown();

	void setSDLMouseCursor(const std::string& cursor_name);
	bool onSDLEvent(const SDL_Event* event);

	void renderBegin();
	void renderStageBegin(GlViewportConstPtr targetViewport);
	void renderStageEnd();
	void renderUIBegin();
	void renderUIEnd();
	void renderEnd();

	static Renderer* getInstance()
	{
		return m_instance;
	}

	struct SDL_Window* getSDLWindow() const
	{
		return m_sdlWindow;
	}

	float getSDLWindowWidth()
	{
		return (float)m_sdlWindowWidth;
	}

	float getSDLWindowHeight()
	{
		return (float)m_sdlWindowHeight;
	}

	float getSDLWindowAspectRatio()
	{
		return (float)m_sdlWindowWidth / (float)m_sdlWindowHeight;
	}

	bool getIsRenderingStage()
	{
		return m_isRenderingStage;
	}

	bool getIsRenderingUI()
	{
		return m_isRenderingUI;
	}

	GlViewportConstPtr getRenderingViewport() const { return m_renderingViewport; }
	class GlStateStack* getGlStateStack() const { return m_glStateStack; }

	class GlLineRenderer* getLineRenderer() const { return m_lineRenderer; }
	class GlTextRenderer* getTextRenderer() const { return m_textRenderer; }
	std::unique_ptr<class GlModelResourceManager>& getModelResourceManager() { return m_modelResourceManager; }

private:
	bool m_sdlInitialized;

	struct SDL_Cursor* cursor_default = nullptr;
	struct SDL_Cursor* cursor_move = nullptr;
	struct SDL_Cursor* cursor_pointer = nullptr;
	struct SDL_Cursor* cursor_resize = nullptr;
	struct SDL_Cursor* cursor_cross = nullptr;
	struct SDL_Cursor* cursor_text = nullptr;
	struct SDL_Cursor* cursor_unavailable = nullptr;

	struct SDL_Window* m_sdlWindow;
	int m_sdlWindowWidth, m_sdlWindowHeight;

	void* m_glContext;
	GlViewportPtr m_uiViewport;
	GlViewportConstPtr m_renderingViewport;
	class GlStateStack* m_glStateStack;

	class GlLineRenderer* m_lineRenderer;
	class GlTextRenderer* m_textRenderer;

	std::unique_ptr< class GlModelResourceManager > m_modelResourceManager;

	std::unique_ptr< class GlRmlUiRender > m_rmlUiRenderer;

	bool m_isRenderingStage;
	bool m_isRenderingUI;

	// OpenGL shader program cache
	std::unique_ptr< class GlShaderCache > m_shaderCache;

	static Renderer* m_instance;
};
