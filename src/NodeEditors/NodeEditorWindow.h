#pragma once

//-- includes -----
#include "SdlFwd.h"
#include "IGlWindow.h"
#include "EditorNodeConstants.h"
#include "NodeEditorFwd.h"
#include "RendererFwd.h"

#include "imgui.h"
#include "imnodes.h"

#include <chrono>
#include <memory>
#include <vector>

//-- definitions -----

class NodeEditorWindow : public IGlWindow
{
public:
	NodeEditorWindow();
	~NodeEditorWindow();

	void update();

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
	void renderMainFrame();
	void renderBottomPanel();
	void renderRightPanel();

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

	ImNodesPinShape BeginPin(EditorPinPtr pin, float alpha);
	void EndPin();
	void InputPin(EditorNodePtr node, EditorPinPtr pin);
	void OutputPin(EditorNodePtr node, EditorPinPtr pin);

	EditorPinPtr GetConnectedPin(EditorNodePtr node, EditorLinkPtr link);
	// The target block/image node may go through multiple program/ping-pong nodes
	// before it is linked to the input pin
	void GetInputTargetNode(EditorNodePtr& connectedNode, EditorPinType type, int index);

	void ExecuteProgramNode(EditorProgramNodePtr progNode);


	// TODO: Move to Nodes
	EditorPinPtr AllocPin(const struct GlProgramUniform& uniform);
	EditorPinPtr AllocPin(const class GlShaderVar& var);

	EditorProgramNodePtr CreateProgramNodePtr(int progId, const ImVec2& pos);
	void CreateProgramNode(int progId, const ImVec2& pos);
	void UpdateProgramNode(int nodeId, int progId);
	void SetProgramNodeFramebuffer(EditorProgramNodePtr node, int framebufferId);

	void CreateBlockNode(const ImVec2& pos, int pinId = -1);
	void CreateTextureNode(int textureId, const ImVec2& pos);
	void CreateImageNode(const ImVec2& pos);

	void CreatePingPongNode(const ImVec2& pos,
							EditorPingPongNodeType type = EditorPingPongNodeType::BUFFER);
	void UpdatePingPongNode(int nodeId, EditorPingPongNodeType type);

	void CreateTimeNode(const ImVec2& pos);
	void CreateMousePosNode(const ImVec2& pos);

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
	ImVec2 m_HangPos;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
	bool m_IsPlaying= false;
	bool m_OnInit= false;
	bool m_PingPongSwap;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};
