#pragma once
#include "Windows/NodeEditorWindow.h"

class CompositorNodeEditorWindow : public NodeEditorWindow
{
public:
	CompositorNodeEditorWindow();

	virtual NodeGraphFactoryPtr getNodeGraphFactory() const override;
	virtual bool saveGraph(bool bShowFileDialog) override;
};