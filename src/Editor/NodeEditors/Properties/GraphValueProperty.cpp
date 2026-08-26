#include "GraphValueProperty.h"
#include "NodeEditorState.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Graphs/NodeGraph.h"
#include "Nodes/VariableNode.h"

void GraphValueProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto variableNode= m_ownerGraph->createTypedNode<VariableNode>(editorState);

	// Set this as the source value property for the new node
	auto self= std::static_pointer_cast<GraphValueProperty>(shared_from_this());
	variableNode->setValueSource(self);
}

void GraphValueProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locLabel("graphProperties.variableHeader")))
	{
		// Name
		MkGui::drawStaticTextProperty(editorState.styleManager->getStyle("node_editor_property_value"),
									  locText("graphProperties.name"), getName());

		// Variable Default
		editorRenderValue(editorState);
	}
}