#include "TextureNode.h"
#include "GlTexture.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"
#include "Properties/GraphArrayProperty.h"

#include "imgui.h"
#include "imnodes.h"

TextureNode::TextureNode()
	: Node()
{}

TextureNode::TextureNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
{
	if (ownerGraph)
	{
		m_textureArrayProperty = ownerGraph->getTypedPropertyByName<TextureArrayProperty>("textures");
		ownerGraph->OnPropertyModifed += MakeDelegate(this, &TextureNode::onGraphPropertyChanged);
	}
}

TextureNode::~TextureNode()
{
	m_textureArrayProperty = nullptr;
	if (m_ownerGraph)
	{
		m_ownerGraph->OnPropertyModifed -= MakeDelegate(this, &TextureNode::onGraphPropertyChanged);
	}
}

bool TextureNode::evaluateNode(NodeEvaluator& evaluator)
{
	TexturePinPtr outPin= getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_target);
	}

	return true;
}


void TextureNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

void TextureNode::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	// Texture
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	ImGui::Image((void*)(intptr_t)m_target->getGlTextureId(), ImVec2(100, 100));
	ImGui::SameLine();

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	editorRenderPopNodeStyle(editorState);
}

void TextureNode::onGraphPropertyChanged(t_graph_property_id id)
{
	if (m_textureArrayProperty && m_textureArrayProperty->getId() == id)
	{
		auto textureArray = m_textureArrayProperty->getArray();
		if (std::find(textureArray.begin(), textureArray.end(), m_target) == textureArray.end())
		{
			setTexture(GlTexturePtr());
		}
	}
}

// -- TextureNode Factory -----
NodePtr TextureNodeFactory::createNode(const NodeEditorState* editorState) const
{
	// Create the node and pins
	TextureNodePtr node = std::make_shared<TextureNode>();
	TexturePinPtr outputPin = node->addPin<TexturePin>("texture", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}