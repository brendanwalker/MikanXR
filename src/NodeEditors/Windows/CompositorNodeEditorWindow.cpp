//-- includes -----
#include "App.h"
#include "GlFrameCompositor.h"
#include "CompositorNodeEditorWindow.h"
#include "Graphs/CompositorNodeGraph.h"

CompositorNodeEditorWindow::CompositorNodeEditorWindow() 
	: NodeEditorWindow()
{
}

NodeGraphFactoryPtr CompositorNodeEditorWindow::getNodeGraphFactory() const
{
	return std::make_shared<CompositorNodeGraphFactory>();
}

bool CompositorNodeEditorWindow::saveGraph(bool bShowFileDialog)
{
	if (NodeEditorWindow::saveGraph(bShowFileDialog))
	{
		GlFrameCompositor* frameCompositor= App::getInstance()->getFrameCompositor();

		frameCompositor->setCompositorGraphAssetPath(m_editorState.nodeGraphPath, true);
	}

	return false;
}
