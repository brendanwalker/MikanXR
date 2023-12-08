#include "TextureNode.h"
#include "GlTexture.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"

#include "imgui.h"
#include "imnodes.h"

TextureNode::TextureNode()
	: Node()
{}

TextureNode::TextureNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{}

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