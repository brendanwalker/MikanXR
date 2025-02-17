//-- includes -----
#include "App.h"
#include "AssetReference.h"
#include "GlFrameCompositor.h"
#include "CompositorNodeEditorWindow.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorUI.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "IconsForkAwesome.h"

CompositorNodeEditorWindow::CompositorNodeEditorWindow() : NodeEditorWindow()
{
}

// -- IMkWindow ----
bool CompositorNodeEditorWindow::startup()
{
	// Tell the frame compositor to create a texture for the editor compositor to write to
	GlFrameCompositor* frameCompositor= MainWindow::getInstance()->getFrameCompositor();
	frameCompositor->setCompositorEvaluatorWindow(eCompositorEvaluatorWindow::editorWindow);

	// Start the node editor window
	if (!NodeEditorWindow::startup())
	{
		return false;
	}

	// Load the graph from the asset path on the main window's frame compositor (if any)
	auto graphAssetPath = frameCompositor->getCompositorGraphAssetPath();
	if (!graphAssetPath.empty() && !loadGraph(graphAssetPath))
	{
		return false;
	}

	// Create a new graph if none was loaded
	if (!m_editorState.nodeGraph)
	{
		newGraph();
	}

	return true;
}

void CompositorNodeEditorWindow::update(float deltaSeconds)
{
	NodeEditorWindow::update(deltaSeconds);

	if (m_isRunningCompositor)
	{
		MainWindow* mainWindow = MainWindow::getInstance();

		if (mainWindow != nullptr)
		{
			VideoSourceViewPtr videoSourceView = mainWindow->getFrameCompositor()->getVideoSource();

			NodeEvaluator evaluator = {};
			evaluator
				.setCurrentWindow(this)
				.setDeltaSeconds(deltaSeconds)
				.setCurrentVideoSourceView(videoSourceView);

			auto node_graph = std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
			if (!node_graph->compositeFrame(evaluator))
			{
				m_lastNodeEvalErrors = evaluator.getErrors();
			}
		}
	}
}

void CompositorNodeEditorWindow::shutdown()
{
	// Tell the frame compositor to free the editor compositor texture
	MainWindow* mainWindow= MainWindow::getInstance();
	if (mainWindow != nullptr)
	{
		GlFrameCompositor* frameCompositor = mainWindow->getFrameCompositor();
		frameCompositor->setCompositorEvaluatorWindow(eCompositorEvaluatorWindow::mainWindow);
	}

	NodeEditorWindow::shutdown();
}

// -- CompositorNodeEditorWindow ----
NodeGraphFactoryPtr CompositorNodeEditorWindow::getNodeGraphFactory() const
{
	return std::make_shared<CompositorNodeGraphFactory>();
}

void CompositorNodeEditorWindow::onNodeGraphCreated()
{
	NodeEditorWindow::onNodeGraphCreated();

	MainWindow* mainWindow = MainWindow::getInstance();
	GlFrameCompositor* frameCompositor = mainWindow->getFrameCompositor();

	// Point the compositor to the editor window writable compositor texture shared from the main window
	// This will allow the main window to editor compositor graph changes in real time
	auto nodeGraph = std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
	nodeGraph->setExternalCompositedFrameTexture(frameCompositor->getEditorWritableFrameTexture());
}

bool CompositorNodeEditorWindow::saveGraph(bool bShowFileDialog)
{
	if (NodeEditorWindow::saveGraph(bShowFileDialog))
	{
		GlFrameCompositor* frameCompositor= MainWindow::getInstance()->getFrameCompositor();

		frameCompositor->setCompositorGraphAssetPath(m_editorState.nodeGraphPath);
	}

	return false;
}

void CompositorNodeEditorWindow::handleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories =
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef =
			NodeEditorUI::receiveTypedDragDropPayload<AssetReference>(
				factory->getAssetRefClassName()))
		{
			assetRef->editorHandleGraphVariablesDragDrop(editorState);
			return;
		}
	}
}

void CompositorNodeEditorWindow::handleMainFrameDragDrop(const NodeEditorState& editorState)
{
	std::vector<GraphPropertyFactoryPtr> validPropertyFactories= 
		getNodeGraph()->editorGetValidPropertyFactories(editorState);
	for (auto factory : validPropertyFactories)
	{
		if (auto property =
			NodeEditorUI::receiveTypedDragDropPayload<GraphProperty>(
				factory->getGraphPropertyClassName()))
		{
			property->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}

	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories =
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef =
			NodeEditorUI::receiveTypedDragDropPayload<AssetReference>(
				factory->getAssetRefClassName()))
		{
			assetRef->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}
}

void CompositorNodeEditorWindow::renderToolbar()
{
	ImGui::PushFont(m_BigIconFont);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

	ImGui::BeginChild("Toolbar", ImVec2(ImGui::GetContentRegionAvail().x, 40));

	ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 30) * 0.5f);

	if (ImGui::Button(ICON_FK_FLOPPY_O "   Save", ImVec2(0, 30)))
	{
		saveGraph(false);
	}

	// Editor Control
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

		ImGui::SameLine();
		ImGui::BeginChild("EditorControl", ImVec2(70, 30), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::SetCursorPosY((ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()) * 0.5f);

		if (m_isRunningCompositor)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
			if (ImGui::SmallButton(ICON_FK_STOP))
			{
				m_isRunningCompositor = false;
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
			if (ImGui::SmallButton(ICON_FK_PLAY))
			{
				m_isRunningCompositor = true;
			}
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
		if (ImGui::SmallButton(ICON_FK_UNDO))
		{
			undo();
		}
		ImGui::PopStyleColor();

		ImGui::EndChild();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
	}

	ImGui::EndChild();

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(4);

	ImGui::PopFont();
}
