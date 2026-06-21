#include "ShapeNode.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "ShapeComponent.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Properties/GraphShapeProperty.h"

#include "imgui.h"
#include "imnodes.h"
#include "MkNodesScopedNode.h"

// -- ShapeNodeConfig -----
configuru::Config ShapeNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["shape_property_id"]= shapePropertyId;

	return pt;
}

void ShapeNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	shapePropertyId= pt.get_or<t_graph_property_id>("shape_property_id", -1);
}

// -- ShapeNode -----
ShapeNode::~ShapeNode() { setOwnerGraph(NodeGraphPtr()); }

void ShapeNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted-= MakeDelegate(this, &ShapeNode::onGraphPropertyDeleted);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted+= MakeDelegate(this, &ShapeNode::onGraphPropertyDeleted);
			m_ownerGraph= newOwnerGraph;
		}
	}
}

bool ShapeNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto shapeNodeConfig= std::static_pointer_cast<const ShapeNodeConfig>(nodeConfig);
		t_graph_property_id propId= shapeNodeConfig->shapePropertyId;

		auto shapeProperty= getOwnerGraph()->getTypedPropertyById<GraphShapeProperty>(propId);
		if (shapeProperty)
		{
			setShapeSource(shapeProperty);
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("ShapeNode::loadFromConfig")
				<< "Failed to find Shape property: " << propId << ", on Shape node";
		}
	}

	return false;
}

void ShapeNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	ShapeNodeConfigPtr shapeNodeConfig= std::static_pointer_cast<ShapeNodeConfig>(nodeConfig);
	shapeNodeConfig->shapePropertyId= m_sourceProperty ? m_sourceProperty->getId() : -1;

	Node::saveToConfig(nodeConfig);
}

ShapeComponentPtr ShapeNode::getShapeComponent() const
{
	return m_sourceProperty ? m_sourceProperty->getShapeComponent() : ShapeComponentPtr();
}

void ShapeNode::setShapeSource(GraphShapePropertyPtr inShapeProperty)
{
	m_sourceProperty= inShapeProperty;

	PropertyPinPtr outPin= getFirstPinOfType<PropertyPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(inShapeProperty);
	}
}

bool ShapeNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setShapeSource
	return true;
}

std::shared_ptr<MkNodesScopedColorStyle> ShapeNode::editorRenderMakeNodeStyle(const NodeEditorState& editorState) const
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_TitleBar, IM_COL32(80, 150, 130, 225))
		.push(ImNodesCol_TitleBarHovered, IM_COL32(80, 150, 130, 225))
		.push(ImNodesCol_TitleBarSelected, IM_COL32(80, 150, 130, 225));
	return style;
}

void ShapeNode::editorRenderNode(const NodeEditorState& editorState)
{
	auto nodeStyle= editorRenderMakeNodeStyle(editorState);
	MkNodesScopedNode scopedNode(m_id);

	editorRenderTitle(editorState);
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));
}

std::string ShapeNode::editorGetTitle() const
{
	if (m_sourceProperty)
	{
		ShapeComponentPtr shapeComponent= m_sourceProperty->getShapeComponent();
		if (shapeComponent)
		{
			return shapeComponent->getName();
		}
	}

	return "Shape";
}

void ShapeNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (m_sourceProperty)
	{
		m_sourceProperty->editorRenderPropertySheet(editorState);
	}
}

void ShapeNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setShapeSource(GraphShapePropertyPtr());
	}
}

// -- ShapeNode Factory -----
NodePtr ShapeNodeFactory::createNode(const NodeEditorState& editorState) const
{
	NodePtr node= NodeFactory::createNode(editorState);
	PropertyPinPtr outputPin= node->addPin<PropertyPin>("Shape", eNodePinDirection::OUTPUT);
	outputPin->setPropertyClassName(GraphShapeProperty::k_propertyClassName);
	outputPin->editorSetShowPinName(false);

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
