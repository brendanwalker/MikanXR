#pragma once

//-- includes -----
#include "SdlFwd.h"
#include "EditorNodeFwd.h"
#include "IGlWindow.h"
#include "NodeEditorFwd.h"
#include "RendererFwd.h"

#include <memory>
#include <vector>

//-- definitions -----

class NodeEditorWindow : public IGlWindow
{
public:
	NodeEditorWindow();
	~NodeEditorWindow();

	// -- IGlWindow ----
	virtual bool startup() override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return false; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual GlViewportConstPtr getRenderingViewport() const override { return nullptr; }
	virtual GlStateStack& getGlStateStack() override;
	virtual GlLineRenderer* getLineRenderer() override;
	virtual GlTextRenderer* getTextRenderer() override;

	virtual bool onSDLEvent(const SDL_Event* event)  override;

protected:
	virtual void configImGui();
	virtual void configImNodes();
	virtual void renderUI();
	virtual void pushImGuiStyles();
	virtual void popImGuiStyles();

private:
	enum class SelectedItemType
	{
		NONE,
		PROGRAM,
		FRAMEBUFFER,
		TEXTURE,
		NODES,
		NODE,
		LINK,
		PROGRAM_NODE,
		BUFFER_NODE,
		IMAGE_NODE,
		PINGPONG_NODE
	};
	SelectedItemType m_SelectedItemType= SelectedItemType::NONE;
	int m_SelectedItemId= -1;

	void renderToolbar();
	void renderLeftPanel();

	void DeleteSelectedItem();

	void AddProgram(GlProgramPtr pProgram);
	void DeleteProgram(int ix);

	void AddFramebuffer(GlFrameBufferPtr pFramebuffer);
	void DeleteFramebuffer(int ix);

	void AddTexture(GlTexturePtr pTex);
	void DeleteTexture(int ix);

	void UpdateNodes();
	void UpdatePins();
	void UpdateLinks();
	void DeletePin(EditorPinPtr pin);
	void DeleteNodePinsAndLinks(int id);
	void DeleteNode(int id);
	void DeleteLink(int id, bool checkPingPongNodes = true);
	void CreateLink(int startPinId, int endPinId);

	void SetProgramNodeFramebuffer(EditorProgramNodePtr node, int framebufferId);

private:
	SdlWindowUniquePtr m_sdlWindow;
	GlStateStackUniquePtr m_glStateStack;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont= nullptr;
	struct ImFont* m_BigIconFont= nullptr;

	std::vector<GlProgramPtr> m_Programs;
	std::vector<GlFrameBufferPtr> m_Framebuffers;
	std::vector<GlTexturePtr> m_Textures;

	std::vector<EditorNodePtr> m_Nodes;
	std::vector<EditorPinPtr> m_Pins;
	std::vector<EditorLinkPtr> m_Links;
	int m_StartedLinkPinId;
	bool m_bLinkHanged;
	struct {
		float x, y;
	} m_HangPos;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
	bool m_IsPlaying= false;
	bool m_OnInit= false;
};
