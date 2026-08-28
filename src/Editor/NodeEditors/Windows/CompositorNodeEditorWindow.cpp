//-- includes -----
#include "App.h"
#include "AssetReference.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "CompositorNodeEditorWindow.h"
#include "Logger.h"
#include "LocText.h"
#include "MkGuiScopedChild.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "MkGuiDrawUtils.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "IconsForkAwesome.h"

CompositorNodeEditorWindow::CompositorNodeEditorWindow(App* ownerApp)
	: NodeEditorWindow(ownerApp)
{
}

// -- IEditorWindow ----
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
	m_compositorComponent= compositorComponent;

	// Load the graph from the asset path on the main window's frame compositor (if any)
	auto graphAssetPath= m_compositorComponent->getCompositorDefinition()->getCompositorGraphPath();
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
	auto compositorNodeGraph= std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
	compositorNodeGraph->bindToCompositorComponent(m_compositorComponent);

	// Tell the compositor component about the node graph it's bound to
	m_compositorComponent->setEditorCompositorNodeGraph(compositorNodeGraph);

	return true;
}

void CompositorNodeEditorWindow::onGraphRestored()
{
	if (m_compositorComponent == nullptr)
	{
		return;
	}

	// Rebind the rebuilt graph instance so the compositor evaluates it
	auto compositorNodeGraph= std::static_pointer_cast<CompositorNodeGraph>(m_editorState.nodeGraph);
	compositorNodeGraph->bindToCompositorComponent(m_compositorComponent);
	m_compositorComponent->setEditorCompositorNodeGraph(compositorNodeGraph);
}

void CompositorNodeEditorWindow::update(float deltaSeconds)
{
	m_lastNodeEvalErrors.clear();

	if (m_compositorComponent != nullptr)
	{
		m_lastNodeEvalErrors= m_compositorComponent->getLastNodeEvalErrors();
	}

	NodeEditorWindow::update(deltaSeconds);
}

void CompositorNodeEditorWindow::shutdown()
{
	// Tell the frame compositor to free the editor compositor texture
	if (m_compositorComponent)
	{
		m_compositorComponent->setEditorCompositorNodeGraph(nullptr);
		m_compositorComponent= nullptr;
	}

	NodeEditorWindow::shutdown();
}

// -- CompositorNodeEditorWindow ----
NodeGraphFactoryPtr CompositorNodeEditorWindow::getNodeGraphFactory() const
{
	return std::make_shared<CompositorNodeGraphFactory>();
}

bool CompositorNodeEditorWindow::saveGraph(bool bShowFileDialog)
{
	if (NodeEditorWindow::saveGraph(bShowFileDialog))
	{
		m_compositorComponent->setCompositorGraphAssetPath(m_editorState.nodeGraphPath);
		return true;
	}

	return false;
}

void CompositorNodeEditorWindow::handleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories=
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef= MkGui::receiveTypedDragDropPayload<AssetReference>(factory->getAssetRefClassName()))
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
		if (auto property= MkGui::receiveTypedDragDropPayload<GraphProperty>(factory->getGraphPropertyClassName()))
		{
			property->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}

	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories=
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef= MkGui::receiveTypedDragDropPayload<AssetReference>(factory->getAssetRefClassName()))
		{
			assetRef->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}
}

void CompositorNodeEditorWindow::renderMenuBarExtras()
{
	if (ImGui::BeginMenu(locLabel("nodeEditor.compositorMenu")))
	{
		bool bRunning= m_compositorComponent ? !m_compositorComponent->getEditorEvaluationPaused() : true;
		if (ImGui::MenuItem(locLabel("nodeEditor.runCompositor"), nullptr, &bRunning) && m_compositorComponent)
		{
			m_compositorComponent->setEditorEvaluationPaused(!bRunning);
		}
		ImGui::EndMenu();
	}
}

bool CompositorNodeEditorWindow::setCompositorRunning(bool bRunning)
{
	if (m_compositorComponent == nullptr)
	{
		return false;
	}

	m_compositorComponent->setEditorEvaluationPaused(!bRunning);
	return true;
}

bool CompositorNodeEditorWindow::isCompositorRunning() const
{
	return m_compositorComponent != nullptr && !m_compositorComponent->getEditorEvaluationPaused();
}
