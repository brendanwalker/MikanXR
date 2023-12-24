#pragma once

#include "GraphArrayProperty.h"

class GraphVariableListConfig : public GraphArrayPropertyConfig
{
public:
	GraphVariableListConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string variableClassName;
};
using GraphVariableListConfigPtr = std::shared_ptr<GraphVariableListConfig>;
using GraphVariableListConfigConstPtr = std::shared_ptr<const GraphVariableListConfig>;

class GraphVariableList : public GraphArrayProperty
{
public:
	GraphVariableList() = default;

	virtual const std::string& getClassName() const override { return "GraphVariableList"; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

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

using GraphVariableListFactory= TypedGraphPropertyFactory<GraphVariableList, GraphVariableListConfig>;