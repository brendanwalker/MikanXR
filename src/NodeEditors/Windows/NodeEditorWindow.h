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
#include "Graphs/GraphObjectSelection.h"
#include "Properties/GraphArrayProperty.h"

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

	virtual void save();
	virtual void undo();

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

	virtual void renderMainFrame();
	virtual void renderContextMenu(const class NodeEditorState& editorState);
	virtual void handleDragDrop(const class NodeEditorState& editorState);
	virtual void renderToolbar();
	virtual void renderGraphVariablesPanel();
	virtual void renderAssetsPanel();
	virtual void renderSelectedObjectPanel();

	virtual void deleteSelectedItem();

	virtual NodeGraphPtr allocateNodeGraph();
	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void onNodeCreated(t_node_id id);
	virtual void onNodeDeleted(t_node_id id);
	virtual void onLinkDeleted(t_node_link_id id);
	virtual void onGraphPropertyCreated(t_graph_property_id id) {}
	virtual void onGraphPropertyModified(t_graph_property_id id) {}
	virtual void onGraphPropertyDeleted(t_graph_property_id id) {}

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
	GraphAssetListPropertyPtr m_assetReferencesList;
	std::vector<GraphVariableListPtr> m_variableLists;

	GraphObjectSelection m_objectSelection;

	// OpenGL shader program cache
	GlShaderCacheUniquePtr m_shaderCache;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
};
