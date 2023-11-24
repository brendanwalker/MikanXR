#pragma once

#include "imgui.h"

class NodeEditorState
{
public:
	int startedLinkPinId;
	bool bLinkHanged;
	ImVec2 hangPos;
};
