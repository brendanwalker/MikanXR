#include "CompositorNodeGraph.h"

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

// -- CompositorNodeGraphFactory -----
NodeGraphPtr CompositorNodeGraphFactory::allocateNodeGraph() const
{
	return std::make_shared<CompositorNodeGraph>();
}

// -- CompositorNodeGraph -----
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

	// Add graph properties
	createTypedProperty<GraphVariableList>("materials")->assignFactory<GraphMaterialPropertyFactory>();
	createTypedProperty<GraphVariableList>("models")->assignFactory<GraphModelPropertyFactory>();
	createTypedProperty<GraphVariableList>("textures")->assignFactory<GraphTexturePropertyFactory>();
}