#include "ModelNode.h"
#include "GlMaterial.h"
#include "GlTexture.h"
#include "Logger.h"
#include "ModelAssetReference.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Properties/GraphModelProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- ModelNodeConfig -----
configuru::Config ModelNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["model_property_id"] = modelPropertyId;

	return pt;
}

void ModelNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	modelPropertyId = pt.get_or<t_graph_property_id>("model_property_id", -1);
}

// -- ModelNode -----
ModelNode::~ModelNode()
{
	setOwnerGraph(NodeGraphPtr());
}

void ModelNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted -= MakeDelegate(this, &ModelNode::onGraphPropertyDeleted);
			m_ownerGraph = nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted += MakeDelegate(this, &ModelNode::onGraphPropertyDeleted);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

bool ModelNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto modelNodeConfig = std::static_pointer_cast<const ModelNodeConfig>(nodeConfig);
		t_graph_property_id propId = modelNodeConfig->modelPropertyId;

		auto materialProperty = getOwnerGraph()->getTypedPropertyById<GraphModelProperty>(propId);
		if (materialProperty)
		{
			setModelSource(materialProperty);
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("ModelNode::loadFromConfig")
				<< "Failed to find model property: " << propId
				<< ", on model node";
		}
	}

	return false;
}

void ModelNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto materialNodeConfig = std::static_pointer_cast<ModelNodeConfig>(nodeConfig);
	materialNodeConfig->modelPropertyId = m_sourceProperty ? m_sourceProperty->getId() : -1;

	Node::saveToConfig(nodeConfig);
}

GlRenderModelResourcePtr ModelNode::getModelResource() const
{
	return m_sourceProperty ? m_sourceProperty->getModelResource() : GlRenderModelResourcePtr();
}

void ModelNode::setModelSource(GraphModelPropertyPtr inModelProperty)
{
	m_sourceProperty = inModelProperty;

	PropertyPinPtr outPin = getFirstPinOfType<PropertyPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_sourceProperty);
	}
}

bool ModelNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setMaterialSource

	return true;
}

void ModelNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string ModelNode::editorGetTitle() const
{
	if (m_sourceProperty)
	{
		auto assetRef = m_sourceProperty->getModelAssetReference();
		if (assetRef)
		{
			return assetRef->getShortName();
		}
		else
		{
			return m_sourceProperty->getName();
		}
	}
	else
	{
		return "Empty Model";
	}

}

void ModelNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr textureResource = m_sourceProperty->getModelAssetReference()->getPreviewTexture();
	uint32_t glTextureId = textureResource ? textureResource->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void ModelNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setModelSource(GraphModelPropertyPtr());
	}
}

// -- ModelNode Factory -----
NodePtr ModelNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	auto outputPin = node->addPin<PropertyPin>("model", eNodePinDirection::OUTPUT);
	outputPin->setPropertyClassName(GraphModelProperty::k_propertyClassName);
	outputPin->editorSetShowPinName(false);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}