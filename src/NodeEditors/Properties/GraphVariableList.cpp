#include "GraphVariableList.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"

GraphPropertyPtr GraphVariableList::addNewVariable(
	const NodeEditorState* editorState,
	const std::string& name)
{
	NodeGraphPtr ownerGraph= getOwnerGraph();

	GraphPropertyPtr variable= m_factory->allocateProperty();
	if (variable)
	{
		// Init variable
		variable->setOwnerGraph(ownerGraph);
		variable->setId(ownerGraph->allocateId());
		variable->setName(name);

		// Register with the graph
		ownerGraph->addProperty(variable);

		// Register with the array
		addProperty(variable);
	}

	return variable;
}

bool GraphVariableList::deleteVariableByIndex(const class NodeEditorState* editorState, int elementIndex)
{
	if (elementIndex >= 0 && elementIndex < (int)m_array.size())
	{
		GraphPropertyPtr variable= m_array[elementIndex];

		// Unregister with the array
		removeProperty(variable);

		// Remove from the graph
		return m_ownerGraph->deletePropertyById(variable->getId());
	}

	return false;
}