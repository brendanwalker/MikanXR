#pragma once

//-- includes -----
#include "AssetFwd.h"
#include "SdlFwd.h"
#include "ISdlMkWindow.h"
#include "NodeEditorFwd.h"
#include "NodeFwd.h"
#include "NodeEditorState.h"
#include "MikanRendererFwd.h"

#include "Graphs/GraphObjectSelection.h"
#include "Graphs/NodeError.h"

#include "Properties/GraphArrayProperty.h"

#include "imgui.h"
#include "imnodes.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

//-- definitions -----
class NodeEditorWindow : public ISdlMkWindow
{
public:
	NodeEditorWindow();
	~NodeEditorWindow();

	inline NodeGraphPtr getNodeGraph() const { return m_editorState.nodeGraph; }
	inline const std::filesystem::path& getNodeGraphPath() const { return m_editorState.nodeGraphPath; }
	virtual NodeGraphFactoryPtr getNodeGraphFactory() const;

	virtual void newGraph();
	virtual bool loadGraph(const std::filesystem::path& path);
	virtual bool saveGraph(bool bShowFileDialog);
	virtual void undo();

	// -- IMkWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return false; }
	virtual bool getIsRenderingUI() const override { return m_isRenderingUI; }

	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }
	virtual MkStateStack& getMkStateStack() override;
	virtual IMkLineRenderer* getLineRenderer() override;
	virtual IMkTextRenderer* getTextRenderer() override;
	virtual MikanModelResourceManager* getModelResourceManager() override;
	virtual IMkShaderCache* getShaderCache() override;
	virtual IMkTextureCache* getTextureCache() override;
	virtual SdlWindow& getSdlWindow() override;

	virtual bool onSDLEvent(const SDL_Event* event) override;

protected:
	virtual void configImGui();
	virtual void configImNodes();
	virtual void renderUI();
	virtual void pushImGuiStyles();
	virtual void popImGuiStyles();

	virtual void renderMainFrame();
	virtual void renderNodeEvalErrors();
	virtual void handleGraphVariablesDragDrop(const class NodeEditorState& editorState) {}
	virtual void handleMainFrameDragDrop(const class NodeEditorState& editorState) {}
	virtual void renderMainFrameContextMenu(const class NodeEditorState& editorState);
	virtual void renderToolbar();
	virtual void renderGraphVariablesPanel();
	virtual void renderNewGraphVariablesContextMenu(const NodeEditorState& editorState);
	virtual void renderAssetsPanel();
	virtual void renderSelectedObjectPanel();

	virtual void deleteSelectedItem();

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
	MkStateStackUniquePtr m_MkStateStack;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont= nullptr;
	struct ImFont* m_BigIconFont= nullptr;

	NodeEditorState m_editorState;

	GraphObjectSelection m_objectSelection;

	// Models loaded by the shader graph
	MikanModelResourceManagerUniquePtr m_modelResourceManager;

	// OpenGL shader program cache
	MikanShaderCacheUniquePtr m_shaderCache;

	// OpenGL texture cache
	MikanTextureCacheUniquePtr m_textureCache;

	// Errors that occurred during the last graph evaluation
	std::vector<NodeEvaluationError> m_lastNodeEvalErrors;

	bool m_imguiSDLBackendInitialised = false;
	bool m_imguiOpenGLBackendInitialised= false;
	bool m_isRenderingUI= false;
};
