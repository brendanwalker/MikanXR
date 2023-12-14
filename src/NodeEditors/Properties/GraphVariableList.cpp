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
		m_array.push_back(variable);
		notifyPropertyModified();
	}

	return variable;
}

bool GraphVariableList::deleteVariableByIndex(const class NodeEditorState* editorState, int elementIndex)
{
	if (elementIndex >= 0 && elementIndex < (int)m_array.size())
	{
		GraphPropertyPtr variable= m_array[elementIndex];

		if (m_ownerGraph->OnPropertyDeleted)
			m_ownerGraph->OnPropertyDeleted(variable->getId());

		m_array.erase(m_array.begin() + elementIndex);

		return true;
	}

	return false;
}