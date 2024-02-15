#include "StencilNode.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "StencilComponent.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Properties/GraphStencilProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- StencilNodeConfig -----
configuru::Config StencilNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["stencil_property_id"] = stencilPropertyId;

	return pt;
}

void StencilNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	stencilPropertyId = pt.get_or<t_graph_property_id>("stencil_property_id", -1);
}

// -- StencilNode -----
StencilNode::~StencilNode()
{
	setOwnerGraph(NodeGraphPtr());
}

void StencilNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted -= MakeDelegate(this, &StencilNode::onGraphPropertyDeleted);
			m_ownerGraph = nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted += MakeDelegate(this, &StencilNode::onGraphPropertyDeleted);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

bool StencilNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto stencilNodeConfig = std::static_pointer_cast<const StencilNodeConfig>(nodeConfig);
		t_graph_property_id propId= stencilNodeConfig->stencilPropertyId;

		auto StencilProperty= getOwnerGraph()->getTypedPropertyById<GraphStencilProperty>(propId);
		if (StencilProperty)
		{
			setStencilSource(StencilProperty);
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("StencilNode::loadFromConfig")
				<< "Failed to find Stencil property: " << propId
				<< ", on Stencil node";
		}
	}

	return false;
}

void StencilNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	StencilNodeConfigPtr stencilNodeConfig = std::static_pointer_cast<StencilNodeConfig>(nodeConfig);
	stencilNodeConfig->stencilPropertyId= m_sourceProperty ? m_sourceProperty->getId() : -1;

	Node::saveToConfig(nodeConfig);
}

StencilComponentPtr StencilNode::getStencilComponent() const
{
	return m_sourceProperty ? m_sourceProperty->getStencilComponent() : StencilComponentPtr();
}

void StencilNode::setStencilSource(GraphStencilPropertyPtr inStencilProperty) 
{ 
	m_sourceProperty = inStencilProperty; 

	PropertyPinPtr outPin = getFirstPinOfType<PropertyPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(inStencilProperty);
	}
}

bool StencilNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setStencilSource

	return true;
}

void StencilNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

void StencilNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	//TODO - Preview Texture from stencil model
	//ImGui::Dummy(ImVec2(1.0f, 0.5f));
	//GlTexturePtr textureResource = m_sourceProperty->getStencilAssetReference()->getPreviewTexture();
	//uint32_t glTextureId = textureResource ? textureResource->getGlTextureId() : 0;
	//ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	//ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

std::string StencilNode::editorGetTitle() const
{
	if (m_sourceProperty)
	{
		StencilComponentPtr stencilComponent= m_sourceProperty->getStencilComponent();
		if (stencilComponent)
		{
			return stencilComponent->getName();
		}
	}

	return "Stencil";
}

void StencilNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (m_sourceProperty)
	{
		m_sourceProperty->editorRenderPropertySheet(editorState);
	}
}

void StencilNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setStencilSource(GraphStencilPropertyPtr());
	}
}

// -- StencilNode Factory -----
NodePtr StencilNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	PropertyPinPtr outputPin = node->addPin<PropertyPin>("Stencil", eNodePinDirection::OUTPUT);
	outputPin->setPropertyClassName(GraphStencilProperty::k_propertyClassName);
	outputPin->editorSetShowPinName(false);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}