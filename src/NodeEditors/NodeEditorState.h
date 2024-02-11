#pragma once

#include "NodeFwd.h"
#include "imgui.h"

#include <filesystem>

class NodeEditorState
{
public:
	NodeEditorState() = default;

	NodeGraphPtr nodeGraph;
	std::filesystem::path nodeGraphPath;
	int startedLinkPinId= -1;
	bool bLinkHanged= false;
	ImVec2 hangPosGridSpace= {}; 
};
