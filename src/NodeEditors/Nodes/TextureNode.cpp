#include "TextureNode.h"
#include "GlTexture.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "TextureAssetReference.h"
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"
#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

// -- TextureNodeConfig -----
configuru::Config TextureNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["texture_property_id"] = texturePropertyId;

	return pt;
}

void TextureNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	texturePropertyId = pt.get_or<t_graph_property_id>("texture_property_id", -1);
}

// -- TextureNode -----
TextureNode::~TextureNode()
{
	// Force de-registration of graph property change delegates
	setOwnerGraph(NodeGraphPtr());
}

void TextureNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted -= MakeDelegate(this, &TextureNode::onGraphPropertyDeleted);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted += MakeDelegate(this, &TextureNode::onGraphPropertyDeleted);
			m_ownerGraph= newOwnerGraph;
		}
	}
}

bool TextureNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto textureNodeConfig = std::static_pointer_cast<const TextureNodeConfig>(nodeConfig);
		t_graph_property_id propId = textureNodeConfig->texturePropertyId;

		auto textureProperty = getOwnerGraph()->getTypedPropertyById<GraphTextureProperty>(propId);
		if (textureProperty)
		{
			setTextureSource(textureProperty);
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("TextureNode::loadFromConfig")
				<< "Failed to find texture property: " << propId
				<< ", on texture node";
		}
	}

	return false;
}

void TextureNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto textureNodeConfig = std::static_pointer_cast<TextureNodeConfig>(nodeConfig);
	textureNodeConfig->texturePropertyId = m_sourceProperty ? m_sourceProperty->getId() : -1;

	Node::saveToConfig(nodeConfig);
}

GlTexturePtr TextureNode::getTextureResource() const
{
	return m_sourceProperty ? m_sourceProperty->getTextureResource() : GlTexturePtr();
}

void TextureNode::setTextureSource(GraphTexturePropertyPtr inTextureProperty) 
{ 
	m_sourceProperty= inTextureProperty; 

	auto outPin = getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_sourceProperty->getTextureResource());
	}
}

bool TextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	// Only update output in in setTextureSource

	return true;
}

void TextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string TextureNode::editorGetTitle() const
{
	if (m_sourceProperty)
	{
		auto assetRef = m_sourceProperty->getTextureAssetReference();
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
		return "Empty Texture";
	}
}

void TextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	GlTexturePtr textureResource = getTextureResource();
	uint32_t glTextureId= textureResource ? textureResource->getGlTextureId() : 0;
	ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void TextureNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (m_sourceProperty)
	{
		m_sourceProperty->editorRenderPropertySheet(editorState);
	}
}

void TextureNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setTextureSource(GraphTexturePropertyPtr());
	}
}

// -- TextureNode Factory -----
NodePtr TextureNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	auto outputPin = node->addPin<TexturePin>("texture", eNodePinDirection::OUTPUT);
	outputPin->editorSetShowPinName(false);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}