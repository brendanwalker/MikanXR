#include "GraphValueProperty.h"
#include "NodeEditorUI.h"
#include "Graphs/NodeGraph.h"
#include "Nodes/VariableNode.h"

void GraphValueProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto variableNode = m_ownerGraph->createTypedNode<VariableNode>(editorState);

	// Set this as the source value property for the new node
	auto self = std::static_pointer_cast<GraphValueProperty>(shared_from_this());
	variableNode->setValueSource(self);
}

void GraphValueProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Variable"))
	{
		// Name
		NodeEditorUI::DrawStaticTextProperty("Name", getName());

		// Variable Default
		editorRenderValue(editorState);
	}
}