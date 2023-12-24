#include "GraphVariableList.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"

// -- GraphVariableListConfig -----
configuru::Config GraphVariableListConfig::writeToJSON()
{
	configuru::Config pt = GraphArrayPropertyConfig::writeToJSON();

	pt["variable_class_name"] = variableClassName;

	return pt;
}

void GraphVariableListConfig::readFromJSON(const configuru::Config& pt)
{
	variableClassName= pt.get_or<std::string>("variable_class_name", "");

	GraphArrayPropertyConfig::readFromJSON(pt);
}

// -- GraphVariableList -----
bool GraphVariableList::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	NodeGraphPtr graph = getOwnerGraph();

	if (GraphArrayProperty::loadFromConfig(propConfig, graphConfig))
	{
		auto varListConfig= std::static_pointer_cast<const GraphVariableListConfig>(propConfig);

		m_factory= graph->getPropertyFactory(varListConfig->variableClassName);
		if (m_factory)
		{
			return true;
		}
		else
		{
			MIKAN_LOG_ERROR("GraphVariableList::loadFromConfig")
				<< "Failed to find factory for variable class name: " << varListConfig->variableClassName
				<< ", on array: " << getName();
		}
	}

	return false;
}

void GraphVariableList::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto varListConfig= std::static_pointer_cast<GraphVariableListConfig>(config);

	varListConfig->variableClassName= m_factory ? m_factory->getGraphPropertyClassName() : "";

	GraphArrayProperty::saveToConfig(config);
}

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