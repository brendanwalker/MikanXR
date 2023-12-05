#pragma once

#include "imgui.h"

class NodeEditorState
{
public:
	int startedLinkPinId= -1;
	bool bLinkHanged= false;
	ImVec2 hangPos= {};
};
