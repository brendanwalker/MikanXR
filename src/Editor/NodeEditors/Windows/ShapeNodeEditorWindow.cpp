//-- includes -----
#include "App.h"
#include "AssetReference.h"
#include "Logger.h"
#include "NodeEditorUI.h"
#include "ShapeComponent.h"
#include "ShapeNodeEditorWindow.h"

#include "Graphs/ShapeNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

ShapeNodeEditorWindow::ShapeNodeEditorWindow(App* ownerApp)
	: NodeEditorWindow(ownerApp)
{
}

// -- IEditorWindow ----
bool ShapeNodeEditorWindow::startup()
{
	if (!NodeEditorWindow::startup())
	{
		return false;
	}

	return true;
}

bool ShapeNodeEditorWindow::bindShapeComponent(ShapeComponentPtr shapeComponent)
{
	assert(shapeComponent);
	m_shapeComponent= shapeComponent;

	// Load the graph from the asset path on the shape component (if any)
	auto graphAssetPath= m_shapeComponent->getShapeComponentDefinition()->getShapeGraphPath();
	if (!graphAssetPath.empty() && !loadGraph(graphAssetPath))
	{
		return false;
	}

	// Create a new graph if none was loaded
	if (!m_editorState.nodeGraph)
	{
		newGraph();
	}

	// Tell the new node graph about the shape component it's bound to
	auto shapeNodeGraph=
		std::static_pointer_cast<ShapeNodeGraph>(m_editorState.nodeGraph);
	shapeNodeGraph->bindToShapeComponent(m_shapeComponent);

	// Tell the shape component about the node graph it's bound to
	m_shapeComponent->setEditorShapeNodeGraph(shapeNodeGraph);

	return true;
}

void ShapeNodeEditorWindow::update(float deltaSeconds)
{
	m_lastNodeEvalErrors.clear();

	if (m_shapeComponent != nullptr)
	{
		m_lastNodeEvalErrors= m_shapeComponent->getLastNodeEvalErrors();
	}

	NodeEditorWindow::update(deltaSeconds);
}

void ShapeNodeEditorWindow::shutdown()
{
	// Tell the shape component to free the editor shape node graph
	if (m_shapeComponent)
	{
		m_shapeComponent->setEditorShapeNodeGraph(nullptr);
		m_shapeComponent= nullptr;
	}

	NodeEditorWindow::shutdown();
}

// -- NodeEditorWindow ----
NodeGraphFactoryPtr ShapeNodeEditorWindow::getNodeGraphFactory() const
{
	return std::make_shared<ShapeNodeGraphFactory>();
}

bool ShapeNodeEditorWindow::saveGraph(bool bShowFileDialog)
{
	if (NodeEditorWindow::saveGraph(bShowFileDialog))
	{
		m_shapeComponent->setShapeGraphAssetPath(m_editorState.nodeGraphPath);
	}

	return false;
}

void ShapeNodeEditorWindow::handleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories=
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef=
				NodeEditorUI::receiveTypedDragDropPayload<AssetReference>(
					factory->getAssetRefClassName()))
		{
			assetRef->editorHandleGraphVariablesDragDrop(editorState);
			return;
		}
	}
}

void ShapeNodeEditorWindow::handleMainFrameDragDrop(const NodeEditorState& editorState)
{
	std::vector<GraphPropertyFactoryPtr> validPropertyFactories=
		getNodeGraph()->editorGetValidPropertyFactories(editorState);
	for (auto factory : validPropertyFactories)
	{
		if (auto property=
				NodeEditorUI::receiveTypedDragDropPayload<GraphProperty>(
					factory->getGraphPropertyClassName()))
		{
			property->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}

	std::vector<AssetReferenceFactoryPtr> validAssetRefFactories=
		getNodeGraph()->editorGetValidAssetRefFactories(editorState);
	for (auto factory : validAssetRefFactories)
	{
		if (auto assetRef=
				NodeEditorUI::receiveTypedDragDropPayload<AssetReference>(
					factory->getAssetRefClassName()))
		{
			assetRef->editorHandleMainFrameDragDrop(editorState);
			return;
		}
	}
}
