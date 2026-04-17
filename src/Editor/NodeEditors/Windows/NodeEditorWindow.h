#pragma once

//-- includes -----
#include "AssetFwd.h"
#include "MkGuiFwd.h"
#include "IEditorWindow.h"
#include "IMkWindowEventListener.h"
#include "MkWindowFwd.h"
#include "MikanRendererFwd.h"
#include "NodeEditorFwd.h"
#include "NodeFwd.h"
#include "NodeEditorState.h"

#include "Graphs/GraphObjectSelection.h"
#include "Graphs/NodeError.h"

#include "Properties/GraphArrayProperty.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

//-- definitions -----
class NodeEditorWindow : public IEditorWindow, public IMkWindowEventListener
{
public:
	NodeEditorWindow(App* ownerApp);
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
	virtual void present() override;
	virtual void shutdown() override;

	virtual const char* getTitle() const override;
	virtual float getWidth() const override;
	virtual float getHeight() const override;
	virtual float getAspectRatio() const override;
	virtual bool getIsRenderingStage() const override { return false; }

	virtual void getMouseScreenPosition(int& outScreenX, int& outScreenY) const override;

	virtual eWindowAPI getWindowAPI() const override;
	virtual void* getNativeWindowHandle() const override;
	virtual IMkGraphicsContextPtr getGraphicsContext() const override;
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }
	virtual void makeContextCurrent() override;
	virtual bool wantsDestroy() const override;
	virtual void setTitle(const std::string& title) override;
	virtual void setSize(int width, int height) override;
	virtual void handleEvents(class IMkWindowEventListener* eventListener) override;
	virtual bool hasMouseFocus() const override;
	virtual bool hasKeyboardFocus() const override;

	virtual MikanModelResourceManager* getModelResourceManager() override;

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

	// -- IEditorWindow
	class MainWindow* getMainWindow() const;
	virtual ProjectManagerPtr getProjectManager() const override;
	virtual class MikanServer* getMikanServer() const override;
	virtual class IMkFontManager* getFontManager() const override;
	virtual class InputManager* getInputManager() const override;
	virtual class OpenCVManager* getOpenCVManager() const override;
	virtual class ClientSourceManager* getClientSourceManager() const override;
	virtual class LocalizationManager* getLocalizationManager() const override;
	virtual class EventBus* getEventBus() const override;
	virtual class MkGuiStyleManager* getMkGuiStyleManager() const override;

	virtual class App* getOwnerApp() const override;
	virtual class AppStage* getCurrentAppStage() const override;
	virtual class AppStage* getParentAppStage() const override;
	virtual class AppStage* pushAppStage(const std::string& appStageName) override;
	virtual void popAppState() override;

protected:
	virtual void updateUI();

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
	class App* m_ownerApp = nullptr;
	IMkWindowPtr m_mkWindow;
	IMkGraphicsContextPtr m_graphicsContext;
	MkGuiContextPtr m_guiContext;
	std::unique_ptr<class MkGuiStyleManager> m_styleManager;

	NodeEditorState m_editorState;

	GraphObjectSelection m_objectSelection;

	// Models loaded by the shader graph
	MikanModelResourceManagerUniquePtr m_modelResourceManager;

	// Errors that occurred during the last graph evaluation
	std::vector<NodeEvaluationError> m_lastNodeEvalErrors;
};
