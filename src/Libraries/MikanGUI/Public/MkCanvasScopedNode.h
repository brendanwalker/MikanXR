#pragma once

#include "MkGuiExport.h"

#include "imgui.h"

// One node on the canvas, Unreal-style: content submitted between beginHeader
// and endHeader becomes the title band, drawn as a rounded color bar across
// the node's full width when the scope closes.
//
//   MkCanvasScopedNode node(nodeId, headerColor);
//   node.beginHeader();
//   ImGui::TextUnformatted(title);
//   node.endHeader();
//   ...pins and content...
class MIKAN_GUI_CLASS MkCanvasScopedNode
{
public:
	MkCanvasScopedNode(int nodeId, const ImVec4& headerColor);
	~MkCanvasScopedNode();

	MkCanvasScopedNode(const MkCanvasScopedNode&)= delete;
	MkCanvasScopedNode& operator=(const MkCanvasScopedNode&)= delete;

	void beginHeader();
	void endHeader();

private:
	int m_nodeId;
	unsigned int m_headerColor;
	ImVec2 m_headerMin;
	ImVec2 m_headerMax;
	bool m_bHasHeader= false;
};
