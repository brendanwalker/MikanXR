//-- includes -----
#include "App.h"
#include "GlFrameCompositor.h"
#include "CompositorNodeEditorWindow.h"
#include "NodeEditorUI.h"

#include "Graphs/CompositorNodeGraph.h"

#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphStencilProperty.h"
#include "Properties/GraphTextureProperty.h"

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

void CompositorNodeEditorWindow::handleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	if (auto materialProperty = 
		NodeEditorUI::receiveTypedDragDropPayload<GraphMaterialProperty>(
			GraphMaterialProperty::k_propertyClassName))
	{
		materialProperty->editorHandleMainFrameDragDrop(editorState);
	}
	else if (auto stencilProperty =
			NodeEditorUI::receiveTypedDragDropPayload<GraphStencilProperty>(
				GraphStencilProperty::k_propertyClassName))
	{
		stencilProperty->editorHandleMainFrameDragDrop(editorState);
	}
	else if (auto textureProperty =
			 NodeEditorUI::receiveTypedDragDropPayload<GraphTextureProperty>(
				 GraphTextureProperty::k_propertyClassName))
	{
		textureProperty->editorHandleMainFrameDragDrop(editorState);
	}
}
