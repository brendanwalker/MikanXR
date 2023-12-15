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

// Nodes
#include "Nodes/DrawTriMeshNode.h"
#include "Nodes/EventNode.h"
#include "Nodes/MousePosNode.h"
#include "Nodes/TextureNode.h"
#include "Nodes/TimeNode.h"

CompositorNodeGraph::CompositorNodeGraph() : NodeGraph()
{
	NodeGraphPtr ownerGraph= shared_from_this();

	m_assetRefFactories.push_back(AssetReferenceFactory::create<ModelAssetReferenceFactory>());
	m_assetRefFactories.push_back(AssetReferenceFactory::create<MaterialAssetReferenceFactory>());
	m_assetRefFactories.push_back(AssetReferenceFactory::create<TextureAssetReferenceFactory>());

	// Nodes this graph can spawn
	m_nodeFactories.push_back(NodeFactory::create<DrawTriMeshNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<EventNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<MousePosNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<TextureNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<TimeNodeFactory>(ownerGraph));

	// Add graph properties
	addTypedProperty<GraphVariableList>("materials")->assignFactory<GraphMaterialPropertyFactory>();
	addTypedProperty<GraphVariableList>("models")->assignFactory<GraphModelPropertyFactory>();
	addTypedProperty<GraphVariableList>("textures")->assignFactory<GraphTexturePropertyFactory>();
}