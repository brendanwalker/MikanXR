#pragma once

//-- includes -----
#include "SdlFwd.h"
#include "IGlWindow.h"
#include "EditorNodeConstants.h"
#include "NodeEditorFwd.h"
#include "NodeFwd.h"
#include "NodeEditorState.h"
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

	void update(float deltaSeconds);

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
		// Graph Elements
		NODES,
		NODE,
		LINK,
		// Assets
		TRI_MESH,
		TEXTURE,
	};
	SelectedItemType m_SelectedItemType= SelectedItemType::NONE;
	int m_SelectedItemId= -1;

	void renderToolbar();
	void renderLeftPanel();
	void renderMainFrame();
	void renderContextMenu(const class NodeEditorState& editorState);
	void renderBottomPanel();
	void renderRightPanel();

	void deleteSelectedItem();

	void addTriangulatedMesh(GlTriangulatedMeshPtr triMesh);
	void deleteTriangulatedMesh(int ix);

	void addTexture(GlTexturePtr texture);
	void deleteTexture(int ix);

protected:
	void onNodeCreated(t_node_id id);
	void onNodeDeleted(t_node_id id);
	void onLinkDeleted(t_node_link_id id);

private:
	SdlWindowUniquePtr m_sdlWindow;
	GlStateStackUniquePtr m_glStateStack;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont= nullptr;
	struct ImFont* m_BigIconFont= nullptr;

	NodeGraphPtr m_nodeGraph;
	TriMeshArrayPropertyPtr m_triMeshArrayProperty;
	TextureArrayPropertyPtr m_textureArrayProperty;
	NodeEditorState m_editorState;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
	bool m_IsPlaying= false;
	bool m_OnInit= false;
};
