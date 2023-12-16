#include "GraphVariableList.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"

GraphVariableList::GraphVariableList()
	: GraphArrayProperty()
{
}

GraphVariableList::GraphVariableList(NodeGraphPtr ownerGraph)
	: GraphArrayProperty(ownerGraph)
{
}

GraphPropertyPtr GraphVariableList::addNewVariable(
	const NodeEditorState* editorState,
	const std::string& name)
{
	GraphPropertyPtr variable= m_factory->createProperty(editorState, name);
	if (variable)
	{
		addProperty(variable);
	}

	return variable;
}

bool GraphVariableList::deleteVariableByIndex(const class NodeEditorState* editorState, int elementIndex)
{
	if (elementIndex >= 0 && elementIndex < (int)m_array.size())
	{
		GraphPropertyPtr variable= m_array[elementIndex];

		removeProperty(variable);

		return m_ownerGraph->deletePropertyById(variable->getId());
	}

	return false;
}