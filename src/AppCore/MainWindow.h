#pragma once

//-- includes -----
#include "AppStage.h"
#include "SdlFwd.h"
#include "IGlWindow.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "SDL_events.h"

#include <memory>
#include <string>
#include <vector>

#include <assert.h>

//-- definitions -----
class MainWindow : public IGlWindow
{
public:
	MainWindow();
	~MainWindow();

	static MainWindow* getInstance()
	{
		return m_instance;
	}

	// -- MainWindow --
	inline class MikanServer* getMikanServer() const { return m_mikanServer; }
	inline ObjectSystemManagerPtr getObjectSystemManager() const { return m_objectSystemManager; }
	inline class FontManager* getFontManager() const { return m_fontManager; }
	inline class VideoSourceManager* getVideoSourceManager() const { return m_videoSourceManager; }
	inline class VRDeviceManager* getVRDeviceManager() const { return m_vrDeviceManager; }
	inline class RmlManager* getRmlManager() const { return m_rmlManager; }
	inline class InputManager* getInputManager() const { return m_inputManager; }
	inline class GlFrameCompositor* getFrameCompositor() const { return m_frameCompositor; }
	inline class OpenCVManager* getOpenCVManager() const { return m_openCVManager; }

	inline AppStage* getCurrentAppStage() const
	{
		return (m_appStageStack.size() > 0) ? m_appStageStack[m_appStageStack.size() - 1] : nullptr;
	}

	inline AppStage* getParentAppStage() const
	{
		return (m_appStageStack.size() > 1) ? m_appStageStack[m_appStageStack.size() - 2] : nullptr;
	}

	template<typename t_app_stage>
	t_app_stage* pushAppStage()
	{
		assert(bAppStackOperationAllowed);
		t_app_stage* appStage = new t_app_stage(this);
		AppStage* parentAppStage =
			m_appStageStack.size() > 0
			? m_appStageStack[m_appStageStack.size() - 1]
			: nullptr;

		m_appStageStack.push_back(appStage);
		m_pendingAppStageOps.push_back({parentAppStage, appStage, AppStageOperation::enter});

		return appStage;
	}

	void processPendingAppStageOps();

	void popAppState()
	{
		assert(bAppStackOperationAllowed);
		AppStage* appStage = getCurrentAppStage();
		if (appStage != nullptr)
		{
			m_appStageStack.pop_back();

			AppStage* parentAppStage =
				m_appStageStack.size() > 0
				? m_appStageStack[m_appStageStack.size() - 1]
				: nullptr;

			m_pendingAppStageOps.push_back({parentAppStage, appStage, AppStageOperation::exit});
		}
	}

	MulticastDelegate<void(AppStage* appStage)> OnAppStageEntered;
	MulticastDelegate<void(AppStage* appStage)> OnAppStageExited;

	// -- IGlWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return m_isRenderingStage; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual GlViewportConstPtr getRenderingViewport() const override;
	virtual GlStateStack& getGlStateStack() override;
	virtual GlLineRenderer* getLineRenderer() override;
	virtual GlTextRenderer* getTextRenderer() override;
	virtual GlModelResourceManager* getModelResourceManager() override;
	virtual GlShaderCache* getShaderCache() override;
	virtual SdlWindow& getSdlWindow() override;

	virtual bool onSDLEvent(const SDL_Event* event) override;

protected:
	void renderBegin();
	void renderStageBegin(GlViewportConstPtr targetViewport);
	void renderStageEnd();
	void renderUIBegin();
	void renderUIEnd();
	void renderEnd();

private:
	// Mikan API Server
	class MikanServer* m_mikanServer = nullptr;

	// Used to blend video with client render targets
	class GlFrameCompositor* m_frameCompositor = nullptr;

	// Input Manager
	class InputManager* m_inputManager = nullptr;

	// Rml UI Manager
	class RmlManager* m_rmlManager = nullptr;

	// Object System manager
	ObjectSystemManagerPtr m_objectSystemManager;

	// OpenCV management
	class OpenCVManager* m_openCVManager;

	// OpenGL/SDL font/baked text string texture cache
	class FontManager* m_fontManager = nullptr;

	// Keeps track of currently connected camera
	class VideoSourceManager* m_videoSourceManager = nullptr;

	// Keeps track of currently connected VR trackers
	class VRDeviceManager* m_vrDeviceManager = nullptr;

	// App Stages
	int m_appStageStackIndex = -1;
	std::vector<AppStage*> m_appStageStack;

	SdlWindowUniquePtr m_sdlWindow;
	GlViewportPtr m_uiViewport;
	GlViewportConstPtr m_renderingViewport;

	GlStateStackUniquePtr m_glStateStack;
	GlLineRendererUniquePtr m_lineRenderer;
	GlTextRendererUniquePtr m_textRenderer;
	GlModelResourceManagerUniquePtr m_modelResourceManager;
	GlRmlUiRenderUniquePtr m_rmlUiRenderer;

	enum class AppStageOperation : int
	{
		enter,
		exit
	};

	struct PendingAppStageOperation
	{
		AppStage* parentAppStage;
		AppStage* appStage;
		AppStageOperation op;
	};
	std::vector<PendingAppStageOperation> m_pendingAppStageOps;
	bool bAppStackOperationAllowed = true;

	bool m_isRenderingStage;
	bool m_isRenderingUI;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	static MainWindow* m_instance;
};
