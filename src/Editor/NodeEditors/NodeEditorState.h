#pragma once

#include "NodeFwd.h"
#include "imgui.h"

#include <filesystem>

class NodeEditorState
{
public:
	NodeEditorState()= default;

	NodeGraphPtr nodeGraph;
	std::filesystem::path nodeGraphPath;
	// The page the canvas shows and new nodes land on (0 is the root page)
	t_graph_page_id currentPageId= 0;
	int startedLinkPinId= -1;
	bool bLinkHanged= false;
	ImVec2 hangPosGridSpace= {};
	class MkGuiStyleManager* styleManager= nullptr;
};
