//-- includes -----
#include "App.h"
#include "AssetReference.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "CompositorNodeEditorWindow.h"
#include "Logger.h"
#include "MkGuiContext.h"
#include "MkGuiScopedStyleVar.h"
#include "MkGuiScopedStyleColor.h"
#include "MkGuiScopedChild.h"
#include "MkGuiScopedFont.h"
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
	NodeEditorWindow::update(deltaSeconds);

	if (m_isRunningCompositor)
	{
		if (m_compositorComponent != nullptr)
		{
			CameraComponentPtr cameraComponent = m_compositorComponent->getCameraComponent();

			NodeEvaluator evaluator = {};
			evaluator
				.setCurrentWindow(this)
				.setDeltaSeconds(deltaSeconds);

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
	MkGuiScopedFont bigIconFont(m_guiContext->getBigIconFont());

	MkGuiScopedStyleVar toolbarStyleVar;
	toolbarStyleVar.push(ImGuiStyleVar_WindowPadding, ImVec2(8, 4))
		.push(ImGuiStyleVar_FramePadding, ImVec2(12, 4))
		.push(ImGuiStyleVar_FrameBorderSize, 0.f);
	MkGuiScopedStyleColor toolbarStyleColor;
	toolbarStyleColor.push(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f))
		.push(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f))
		.push(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f))
		.push(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

	MkGuiScopedChild toolbarChild("Toolbar", ImVec2(ImGui::GetContentRegionAvail().x, 40));

	ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 30) * 0.5f);

	if (ImGui::Button(ICON_FK_FLOPPY_O "   Save", ImVec2(0, 30)))
	{
		saveGraph(false);
	}

	// Editor Control
	{
		MkGuiScopedStyleVar editorControlStyleVar;
		editorControlStyleVar.push(ImGuiStyleVar_ChildRounding, 4.0f)
			.push(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		MkGuiScopedStyleColor editorControlStyleColor;
		editorControlStyleColor.push(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f))
			.push(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f))
			.push(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

		ImGui::SameLine();
		MkGuiScopedChild editorControlChild("EditorControl", ImVec2(70, 30), true, ImGuiWindowFlags_NoScrollbar);
		ImGui::SetCursorPosY((ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()) * 0.5f);

		{
			MkGuiScopedStyleColor playStopColor;
			if (m_isRunningCompositor)
			{
				playStopColor.push(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				if (ImGui::SmallButton(ICON_FK_STOP))
				{
					m_isRunningCompositor = false;
				}
			}
			else
			{
				playStopColor.push(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
				if (ImGui::SmallButton(ICON_FK_PLAY))
				{
					m_isRunningCompositor = true;
				}
			}
		}

		ImGui::SameLine();
		{
			MkGuiScopedStyleColor undoTextColor;
			undoTextColor.push(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			if (ImGui::SmallButton(ICON_FK_UNDO))
			{
				undo();
			}
		}
	}
}
