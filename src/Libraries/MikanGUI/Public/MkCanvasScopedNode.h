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

	// Window-space X of the active node's content origin, for right-aligning
	// rows against a target content width regardless of what a row has
	// already consumed. Valid only while a node scope is open.
	static float getCurrentContentStartX();

private:
	int m_nodeId;
	float m_contentStartX= 0.f;
	MkCanvasScopedNode* m_previousNode= nullptr;
	unsigned int m_headerColor;
	ImVec2 m_headerMin;
	ImVec2 m_headerMax;
	bool m_bHasHeader= false;
};
