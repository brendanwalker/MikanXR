#pragma once

#include "GraphArrayProperty.h"

class GraphVariableList : public GraphArrayProperty
{
public:
	GraphVariableList() = default;

	template <class t_factory_type>
	void assignFactory()
	{
		m_factory = GraphPropertyFactory::createFactory<t_factory_type>();
	}
	inline GraphPropertyFactoryPtr getFactory() const { return m_factory; }

	GraphPropertyPtr addNewVariable(const class NodeEditorState* editorState, const std::string& name);
	bool deleteVariableByIndex(const class NodeEditorState* editorState, int elementIndex);

protected:
	GraphPropertyFactoryPtr m_factory;
};

using GraphVariableListFactory= TypedGraphPropertyFactory<GraphVariableList, GraphArrayPropertyConfig>;