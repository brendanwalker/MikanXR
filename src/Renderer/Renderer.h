#pragma once

//-- includes -----
#include <memory>
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

	bool onSDLEvent(const SDL_Event* event);

	void renderBegin();
	void renderStageBegin();
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

	class GlLineRenderer* getLineRenderer() const { return m_lineRenderer; }
	class GlTextRenderer* getTextRenderer() const { return m_textRenderer; }
	std::unique_ptr<class GlModelResourceManager>& getModelResourceManager() { return m_modelResourceManager; }

	class GlCamera* getCurrentCamera() const;
	class GlCamera* pushCamera();
	void popCamera();

private:
	bool m_sdlInitialized;

	struct SDL_Window* m_sdlWindow;
	int m_sdlWindowWidth, m_sdlWindowHeight;

	void* m_glContext;
	std::vector<class GlCamera*> m_cameraStack;

	class GlLineRenderer* m_lineRenderer;
	class GlTextRenderer* m_textRenderer;

	std::unique_ptr< class GlModelResourceManager > m_modelResourceManager;

	struct ImGuiContext* m_imguiContext;
	bool m_imguiOpenGLBackendInitialised;
	bool m_imguiSDLBackendInitialised;

	bool m_isRenderingStage;
	bool m_isRenderingUI;

	static Renderer* m_instance;
};

bool saveTextureToPNG(class GlTexture* texture, const char* filename);
