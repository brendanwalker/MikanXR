#pragma once

#include "GraphArrayProperty.h"

class GraphVariableList : public GraphArrayProperty
{
public:
	GraphVariableList();
	GraphVariableList(NodeGraphPtr ownerGraph);

	template <class t_factory_type>
	void assignFactory()
	{
		m_factory = std::make_shared<t_factory_type>(m_ownerGraph);
	}
	inline GraphPropertyFactoryPtr getFactory() const { return m_factory; }

	GraphPropertyPtr addNewVariable(const class NodeEditorState* editorState, const std::string& name);
	bool deleteVariableByIndex(const class NodeEditorState* editorState, int elementIndex);

protected:
	GraphPropertyFactoryPtr m_factory;
};