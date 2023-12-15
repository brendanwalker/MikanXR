#pragma once
#include "Windows/NodeEditorWindow.h"

class CompositorNodeEditorWindow : public NodeEditorWindow
{
public:
	CompositorNodeEditorWindow();

	// -- NodeEditorWindow -----
	virtual NodeGraphPtr allocateNodeGraph();
};