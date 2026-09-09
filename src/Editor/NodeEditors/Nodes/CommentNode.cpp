#include "CommentNode.h"
#include "MkCanvasWidgets.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"

#include "imgui.h"

#include <cfloat>
#include <cstring>

namespace
{
const float k_commentTextLineCount= 6.f;
}

// -- CommentNodeConfig -----
configuru::Config CommentNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["text"]= text;
	writeStdArray<float, 4>(pt, "color", color);
	writeStdArray<float, 2>(pt, "size", size);

	return pt;
}

void CommentNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	text= pt.get_or<std::string>("text", "");
	if (pt.has_key("color"))
	{
		readStdArray<float, 4>(pt, "color", color);
	}
	if (pt.has_key("size"))
	{
		readStdArray<float, 2>(pt, "size", size);
	}
}

// -- CommentNode -----
bool CommentNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const CommentNodeConfig>(nodeConfig);

		m_text= config->text;
		m_color= config->color;
		m_size= config->size;
		m_bApplySizeOnDraw= true;

		return true;
	}

	return false;
}

void CommentNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<CommentNodeConfig>(nodeConfig);

	config->text= m_text;
	config->color= m_color;
	config->size= m_size;

	Node::saveToConfig(nodeConfig);
}

void CommentNode::setSize(const std::array<float, 2>& size)
{
	m_size= size;
	m_bApplySizeOnDraw= true;
}

std::string CommentNode::editorGetTitle() const
{
	const size_t lineEnd= m_text.find('\n');
	const std::string firstLine= (lineEnd == std::string::npos) ? m_text : m_text.substr(0, lineEnd);

	return !firstLine.empty() ? firstLine : locText("nodes.commentTitle");
}

ImVec4 CommentNode::editorGetHeaderColor() const { return ImVec4(m_color[0], m_color[1], m_color[2], m_color[3]); }

void CommentNode::editorRenderNode(const NodeEditorState& editorState)
{
	ImVec2 groupSize(m_size[0], m_size[1]);
	MkCanvas::drawCommentNode(m_id, editorGetTitle().c_str(), editorGetHeaderColor(), groupSize, m_bApplySizeOnDraw);
	m_bApplySizeOnDraw= false;

	// The canvas reports the size as resized by the user, which is what the file keeps
	m_size= {groupSize.x, groupSize.y};
}

void CommentNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.commentHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		ImGui::TextUnformatted(locText("nodes.commentColor"));
		ImGui::SameLine((float)propertyStyle->getLabelWidth());
		ImGui::ColorEdit4("##commentColor", m_color.data(), ImGuiColorEditFlags_NoInputs);

		ImGui::TextUnformatted(locText("nodes.commentText"));
		strncpy_s(m_textBuffer, sizeof(m_textBuffer), m_text.c_str(), _TRUNCATE);
		const ImVec2 textSize(-FLT_MIN, ImGui::GetTextLineHeight() * k_commentTextLineCount);
		if (ImGui::InputTextMultiline("##commentText", m_textBuffer, sizeof(m_textBuffer), textSize))
		{
			m_text= m_textBuffer;
		}
	}
}
