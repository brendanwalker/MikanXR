#pragma once

//-- includes -----
#include "AssetFwd.h"
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

typedef int t_selected_item_type;

namespace SelectedItemType
{
	const t_selected_item_type NONE = -1;

	// Graph Elements
	const t_selected_item_type NODES= 0;
	const t_selected_item_type NODE= 1;
	const t_selected_item_type LINK= 2;

	// Assets
	const t_selected_item_type ASSET= 3;
};

//-- definitions -----
class NodeEditorWindow : public IGlWindow
{
public:
	NodeEditorWindow();
	~NodeEditorWindow();

	virtual void update(float deltaSeconds);

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

	t_selected_item_type m_SelectedItemType= SelectedItemType::NONE;
	int m_SelectedItemId= -1;

	virtual void renderMainFrame();
	virtual void renderContextMenu(const class NodeEditorState& editorState);
	virtual void renderDragDrop(const class NodeEditorState& editorState) {}
	virtual void renderToolbar() {}
	virtual void renderGraphVariablesPanel() {}
	virtual void renderAssetsPanel();
	virtual void renderSelectedObjectPanel() {}

	virtual void deleteSelectedItem();

	virtual NodeGraphPtr allocateNodeGraph();
	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void onNodeCreated(t_node_id id);
	virtual void onNodeDeleted(t_node_id id);
	virtual void onLinkDeleted(t_node_link_id id);

	virtual void onAssetReferenceCreated(AssetReferencePtr assetRef) {}
	virtual void onAssetReferenceDeleted(AssetReferencePtr assetRef) {}

protected:
	SdlWindowUniquePtr m_sdlWindow;
	GlStateStackUniquePtr m_glStateStack;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont= nullptr;
	struct ImFont* m_BigIconFont= nullptr;

	NodeGraphPtr m_nodeGraph;
	NodeEditorState m_editorState;
	AssetReferenceArrayPropertyPtr m_assetRefArrayProperty;
	AssetReferenceFactoryList m_assetRefFactoryList;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
};