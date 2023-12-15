//-- includes -----
#include "CompositorNodeEditorWindow.h"
#include "Graphs/CompositorNodeGraph.h"

CompositorNodeEditorWindow::CompositorNodeEditorWindow() : NodeEditorWindow()
{
}

NodeGraphPtr CompositorNodeEditorWindow::allocateNodeGraph()
{
	return std::make_shared<CompositorNodeGraph>();
}