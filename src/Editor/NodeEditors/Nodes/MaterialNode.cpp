#include "MaterialNode.h"
#include "MkMaterial.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MaterialAssetReference.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Properties/GraphMaterialProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- MaterialNodeConfig -----
configuru::Config MaterialNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["material_property_id"] = materialPropertyId;

	return pt;
}

void MaterialNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	materialPropertyId = pt.get_or<t_graph_property_id>("material_property_id", -1);
}

// -- MaterialNode -----
MaterialNode::~MaterialNode()
{
	setOwnerGraph(NodeGraphPtr());
}

void MaterialNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted -= MakeDelegate(this, &MaterialNode::onGraphPropertyDeleted);
			m_ownerGraph = nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted += MakeDelegate(this, &MaterialNode::onGraphPropertyDeleted);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

bool MaterialNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto materialNodeConfig = std::static_pointer_cast<const MaterialNodeConfig>(nodeConfig);
		t_graph_property_id propId= materialNodeConfig->materialPropertyId;

		auto materialProperty= getOwnerGraph()->getTypedPropertyById<GraphMaterialProperty>(propId);
		if (materialProperty)
		{
			setMaterialSource(materialProperty);
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("MaterialNode::loadFromConfig")
				<< "Failed to find material property: " << propId
				<< ", on material node";
		}
	}

	return false;
}

void MaterialNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (m_sourceProperty)
	{
		m_sourceProperty->editorRenderPropertySheet(editorState);
	}
}

void MaterialNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto materialNodeConfig = std::static_pointer_cast<MaterialNodeConfig>(nodeConfig);
	materialNodeConfig->materialPropertyId= m_sourceProperty ? m_sourceProperty->getId() : -1;

	Node::saveToConfig(nodeConfig);
}

MkMaterialConstPtr MaterialNode::getMaterialResource() const
{
	return m_sourceProperty ? m_sourceProperty->getMaterialResource() : MkMaterialConstPtr();
}

void MaterialNode::setMaterialSource(GraphMaterialPropertyPtr inMaterialProperty) 
{ 
	m_sourceProperty = inMaterialProperty; 

	auto outPin = getFirstPinOfType<PropertyPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_sourceProperty);
	}
}

bool MaterialNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output pin in setMaterialSource

	return true;
}

void MaterialNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string MaterialNode::editorGetTitle() const 
{ 
	if (m_sourceProperty)
	{
		auto assetRef= m_sourceProperty->getMaterialAssetReference();
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
		return "Empty Material";
	}
}

void MaterialNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	auto materialAssetRef = m_sourceProperty->getMaterialAssetReference();
	if (materialAssetRef)
	{
		auto previewTexture= materialAssetRef->getPreviewTexture();

		if (previewTexture)
		{
			uint32_t glTextureId = previewTexture->getGlTextureId();
			if (glTextureId != 0)
			{
				ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
				ImGui::SameLine();
			}
		}
	}

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void MaterialNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setMaterialSource(GraphMaterialPropertyPtr());
	}
}

// -- MaterialNode Factory -----
NodePtr MaterialNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	PropertyPinPtr outputPin = node->addPin<PropertyPin>("material", eNodePinDirection::OUTPUT);
	outputPin->setPropertyClassName(GraphMaterialProperty::k_propertyClassName);
	outputPin->editorSetShowPinName(false);
	//TODO: Add vertex definition attribute to the pin

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}