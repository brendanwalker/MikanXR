//-- includes -----
#include "App.h"
#include "AssetReference.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "CompositorNodeEditorWindow.h"
#include "Logger.h"
#include "MkGuiScopedChild.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorUI.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "IconsForkAwesome.h"

CompositorNodeEditorWindow::CompositorNodeEditorWindow(App* ownerApp)
	: NodeEditorWindow(ownerApp)
{
}

// -- IMkWindow ----
bool CompositorNodeEditorWindow::startup()
{
	// Start the node editor window
	if (!NodeEditorWindow::startup())
	{
		return false;
	}

	return true;
}

bool CompositorNodeEditorWindow::bindCompositorComponent(CompositorComponentPtr compositorComponent)
{
	assert(compositorComponent);
	m_compositorComponent = compositorComponent;

	// Tell the frame compositor to create a texture for the editor compositor to write to
	m_compositorComponent->setCompositorEvaluatorWindow(eCompositorEvaluatorWindow::editorWindow);

	// Load the graph from the asset path on the main window's frame compositor (if any)
	auto graphAssetPath = m_compositorComponent->getCompositorDefinition()->getCompositorGraphPath();
	if (!graphAssetPath.empty() && !loadGraph(graphAssetPath))
	{
		return false;
	}

	// Create a new graph if none was loaded
	if (!m_editorState.nodeGraph)
	{
		newGraph();
	}

	// Tell the new node graph about the compositor component it's bound to
	auto compositorNodeGraph = 
		std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
	compositorNodeGraph->bindToCompositorComponent(m_compositorComponent);

	return true;
}

void CompositorNodeEditorWindow::update(float deltaSeconds)
{
	m_lastNodeEvalErrors.clear();

	if (m_isRunningCompositor)
	{
		if (m_compositorComponent != nullptr)
		{
			CameraComponentPtr cameraComponent = m_compositorComponent->getCameraComponent();

			NodeEvaluator evaluator = {};
			evaluator
				.setCurrentGraphicsContext(getGraphicsContext().get())
				.setDeltaSeconds(deltaSeconds);

			auto node_graph = std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
			if (!node_graph->compositeFrame(evaluator))
			{
				m_lastNodeEvalErrors = evaluator.getErrors();
			}
		}
	}

	NodeEditorWindow::update(deltaSeconds);
}

void CompositorNodeEditorWindow::shutdown()
{
	// Tell the frame compositor to free the editor compositor texture
	if (m_compositorComponent)
	{
		m_compositorComponent->setCompositorEvaluatorWindow(eCompositorEvaluatorWindow::mainWindow);
		m_compositorComponent = nullptr;
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

	// Point the compositor to the editor window writable compositor texture shared from the main window
	// This will allow the main window to editor compositor graph changes in real time
	auto nodeGraph = std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
	nodeGraph->setExternalCompositedFrameTexture(m_compositorComponent->getEditorWritableFrameTexture());
}

bool CompositorNodeEditorWindow::saveGraph(bool bShowFileDialog)
{
	if (NodeEditorWindow::saveGraph(bShowFileDialog))
	{
		m_compositorComponent->getCompositorDefinition()->setCompositorGraphPath(
			m_editorState.nodeGraphPath);
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
	MkGuiScopedStyle toolbarStyle(getMkGuiStyleManager()->getStyle("node_editor_toolbar"));

	MkGuiScopedChild toolbarChild("Toolbar", ImVec2(ImGui::GetContentRegionAvail().x, 40));

	ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 30) * 0.5f);

	if (ImGui::Button(ICON_FK_FLOPPY_O "   Save", ImVec2(0, 30)))
	{
		saveGraph(false);
	}

	// Editor Control
	{
		MkGuiScopedStyle controlPanelStyle(getMkGuiStyleManager()->getStyle("node_editor_control_panel"));

		ImGui::SameLine();
		MkGuiScopedChild editorControlChild("EditorControl", ImVec2(70, 30), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::SetCursorPosY((ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()) * 0.5f);

		{
			const char* playStopStyleName = m_isRunningCompositor ? "compositor_stop_button" : "compositor_play_button";
			MkGuiScopedStyle playStopStyle(getMkGuiStyleManager()->getStyle(playStopStyleName));
			if (m_isRunningCompositor)
			{
				if (ImGui::SmallButton(ICON_FK_STOP))
				{
					m_isRunningCompositor = false;
				}
			}
			else
			{
				if (ImGui::SmallButton(ICON_FK_PLAY))
				{
					m_isRunningCompositor = true;
				}
			}
		}

		ImGui::SameLine();
		{
			MkGuiScopedStyle undoStyle(getMkGuiStyleManager()->getStyle("node_editor_undo_button"));
			if (ImGui::SmallButton(ICON_FK_UNDO))
			{
				undo();
			}
		}
	}
}
