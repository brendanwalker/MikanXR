#include "CompositorNodeGraph.h"
#include "Logger.h"

// Assets References
#include "ModelAssetReference.h"
#include "MaterialAssetReference.h"
#include "TextureAssetReference.h"

// Properties
#include "Properties/GraphVariableList.h"
#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphModelProperty.h"
#include "Properties/GraphTextureProperty.h"

// Pins
#include "Pins/MaterialPin.h"
#include "Pins/ModelPin.h"
#include "Pins/TexturePin.h"

// Nodes
#include "Nodes/DrawTriMeshNode.h"
#include "Nodes/EventNode.h"
#include "Nodes/MaterialNode.h"
#include "Nodes/MousePosNode.h"
#include "Nodes/TextureNode.h"
#include "Nodes/TimeNode.h"

// Graph
#include "NodeEditorState.h"
#include "Graphs/NodeEvaluator.h"

#include <easy/profiler.h>

// -- CompositorNodeGraph -----
const std::string CompositorNodeGraph::k_compositeFrameEventName= "OnCompositeFrame";

CompositorNodeGraph::CompositorNodeGraph() : NodeGraph()
{
	NodeGraphPtr ownerGraph= shared_from_this();

	// Assets this graph can reference
	addAssetReferenceFactory<ModelAssetReferenceFactory>();
	addAssetReferenceFactory<MaterialAssetReferenceFactory>();
	addAssetReferenceFactory<TextureAssetReferenceFactory>();

	// Add pin types nodes in this graph can use
	addPinFactory<MaterialPin>();
	addPinFactory<ModelPin>();
	addPinFactory<TexturePin>();

	// Add property types this graph can use
	addPropertyFactory<GraphMaterialPropertyFactory>();
	addPropertyFactory<GraphModelPropertyFactory>();
	addPropertyFactory<GraphTexturePropertyFactory>();

	// Nodes this graph can spawn
	addNodeFactory<DrawTriMeshNodeFactory>();
	addNodeFactory<EventNodeFactory>();
	addNodeFactory<MousePosNodeFactory>();
	addNodeFactory<MaterialNodeFactory>();
	addNodeFactory<TextureNodeFactory>();
	addNodeFactory<TimeNodeFactory>();
}

bool CompositorNodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	bool bSuccess= true;

	if (NodeGraph::loadFromConfig(config))
	{
		m_compositeFrameEventNode= getEventNodeByName(k_compositeFrameEventName);
		if (!m_compositeFrameEventNode)
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::loadFromConfig") 
				<< "Failed to find event node: " << k_compositeFrameEventName;
			bSuccess= false;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("CompositorNodeGraph::loadFromConfig") << "Failed to parse node graph config";
		bSuccess= false;
	}

	return bSuccess;
}

bool CompositorNodeGraph::compositeFrame(NodeEvaluator& evaluator)
{
	EASY_FUNCTION();

	if (m_compositeFrameEventNode)
	{
		evaluator.evaluateFlowPinChain(m_compositeFrameEventNode);
		if (evaluator.getLastErrorCode() != eNodeEvaluationErrorCode::NONE)
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::compositeFrame - Error: ") << evaluator.getLastErrorMessage();
		}
	}
	else
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::invalidNode);
		evaluator.setLastErrorMessage("Missing compositeFrame event node");
	}

	return evaluator.getLastErrorCode() == eNodeEvaluationErrorCode::NONE;
}

// -- CompositorNodeGraphFactory ----
NodeGraphPtr CompositorNodeGraphFactory::initialCreateNodeGraph() const
{
	auto nodeGraph= NodeGraphFactory::initialCreateNodeGraph();
	auto compositorNodeGraph= std::static_pointer_cast<CompositorNodeGraph>(nodeGraph);

	// Add default nodes
	NodeEditorState editorState;
	editorState.nodeGraph= compositorNodeGraph;

	auto compositeFrameNode= compositorNodeGraph->createTypedNode<EventNode>(editorState);
	compositeFrameNode->setName(CompositorNodeGraph::k_compositeFrameEventName);

	// Add default properties
	compositorNodeGraph->createTypedProperty<GraphVariableList>("materials")
						->assignFactory<GraphMaterialPropertyFactory>();
	compositorNodeGraph->createTypedProperty<GraphVariableList>("models")
						->assignFactory<GraphModelPropertyFactory>();
	compositorNodeGraph->createTypedProperty<GraphVariableList>("textures")
						->assignFactory<GraphTexturePropertyFactory>();

	return compositorNodeGraph;
}