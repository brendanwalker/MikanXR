#pragma once

//-- includes -----
#include "AssetFwd.h"
#include "EditorWindow.h"
#include "NodeEditorFwd.h"
#include "NodeFwd.h"
#include "NodeEditorState.h"

#include "Graphs/GraphObjectSelection.h"
#include "Graphs/NodeError.h"
#include "Graphs/NodeGraphHistory.h"
#include "Graphs/NodeGraphLogWriter.h"

#include "Properties/GraphArrayProperty.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "glm/ext/vector_float2.hpp"

//-- definitions -----
class NodeEditorWindow : public EditorWindow
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

	// Schedule undo/redo steps, applied at the top of the next update outside
	// the ImGui frame
	virtual void undo();
	virtual void redo();
	bool canUndo() const { return m_history.canUndo(); }
	bool canRedo() const { return m_history.canRedo(); }
	const NodeGraphHistory& getHistory() const { return m_history; }
	const std::filesystem::path& getGraphLogFilePath() const { return m_logWriter.getLogFilePath(); }

	/// Apply undo (negative) or redo (positive) steps immediately.
	/// Safe only outside the window's ImGui frame; UI paths schedule through
	/// undo()/redo() instead. @returns true if the graph was restored
	bool stepHistory(int steps);

	// -- Automation ----
	// Parked work from the automation server, run at the top of the next
	// update where this window's GL and gui contexts can be made current
	void enqueueAutomationTask(std::function<void()>&& task);

	// Graph mutations for automation tasks (push the needed contexts internally)
	bool automationCreateNode(const std::string& nodeClassName, const glm::vec2& gridPos, t_node_id& outNodeId,
							  std::string& outError);
	bool automationDeleteNode(t_node_id nodeId, std::string& outError);
	bool automationCreateLink(t_node_pin_id startPinId, t_node_pin_id endPinId, t_node_link_id& outLinkId,
							  std::string& outError);
	bool automationDeleteLink(t_node_link_id linkId, std::string& outError);

	// Ask the app to tear this window down at the end of the frame
	void requestClose() { m_bCloseRequested= true; }

	// -- IEditorWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual bool getIsRenderingStage() const override { return false; }
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }
	virtual bool wantsDestroy() const override;

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

protected:
	virtual void updateUI();

	virtual void renderMainFrame();
	virtual void renderNodeEvalErrors();
	virtual void handleGraphVariablesDragDrop(const class NodeEditorState& editorState) {}
	virtual void handleMainFrameDragDrop(const class NodeEditorState& editorState) {}
	virtual void renderMainFrameContextMenu(const class NodeEditorState& editorState);
	void renderMenuBar();
	// Extra menus appended after File/Edit/View (the compositor window's Compositor menu)
	virtual void renderMenuBarExtras() {}
	void buildDefaultDockLayout(unsigned int dockspaceId);
	virtual void renderGraphVariablesPanel();
	virtual void renderNewGraphVariablesContextMenu(const NodeEditorState& editorState);
	virtual void renderAssetsPanel();
	virtual void renderSelectedObjectPanel();
	void renderVariableNameField(GraphPropertyPtr property);

	virtual void deleteSelectedItem();

	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void onNodeCreated(t_node_id id);
	virtual void onNodeDeleted(t_node_id id);
	virtual void onLinkCreated(t_node_link_id id);
	virtual void onLinkDeleted(t_node_link_id id);
	virtual void onPinCreated(t_node_pin_id id);
	virtual void onPinDeleted(t_node_pin_id id);
	virtual void onGraphPropertyCreated(t_graph_property_id id);
	virtual void onGraphPropertyModified(t_graph_property_id id);
	virtual void onGraphPropertyDeleted(t_graph_property_id id);

	virtual void onAssetReferenceCreated(AssetReferencePtr assetRef);
	virtual void onAssetReferenceDeleted(AssetReferencePtr assetRef);

	// Rebind the restored graph to its owning component after an undo/redo
	// rebuilds the graph instance
	virtual void onGraphRestored() {}

	// -- Undo history ----
	void markHistoryCheckpoint() { m_bCheckpointPending= true; }
	void updateHistoryCapture();
	bool restoreGraphSnapshot(const std::string& snapshot);
	void flushAutomationTasks();

	// Open the session log (first graph only) and record the graph baseline
	void logGraphBaseline();

protected:
	NodeEditorState m_editorState;

	GraphObjectSelection m_objectSelection;

	// Errors that occurred during the last graph evaluation
	std::vector<NodeEvaluationError> m_lastNodeEvalErrors;

	// Undo history: whole-graph snapshots committed once per quiescent frame
	NodeGraphHistory m_history;
	bool m_bCheckpointPending= false;
	bool m_bAnyItemActiveLastFrame= false;
	int m_pendingHistorySteps= 0;
	bool m_bApplyingHistory= false;

	// JSONL session log of the graph edits, for post-session diagnosis
	NodeGraphLogWriter m_logWriter;
	int64_t m_nextLogSequenceNumber= 1;

	// Parked automation work (see enqueueAutomationTask)
	std::vector<std::function<void()>> m_automationTasks;

	bool m_bCloseRequested= false;

	// Rename field state for the selected graph variable
	char m_variableNameBuffer[256]= {};
	t_graph_property_id m_variableNameBufferId= -1;

	// Dockable panel visibility, toggled from the View menu (not persisted;
	// the dock geometry itself persists via the window's imgui ini)
	bool m_bShowVariablesPanel= true;
	bool m_bShowAssetsPanel= true;
	bool m_bShowDetailsPanel= true;
};
