#pragma once

#include "ComponentFwd.h"
#include "GraphProperty.h"
#include "ProjectConfigConstants.h"

class GraphShapePropertyConfig : public GraphPropertyConfig
{
public:
	GraphShapePropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eShapeType shapeType;
	std::string shapeName;
};

class GraphShapeProperty : public GraphProperty
{
public:
	GraphShapeProperty() = default;

	inline static const std::string k_propertyClassName = "GraphShapeProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	inline void setShapeComponent(ShapeComponentPtr inComponent) { m_shapeComponent = inComponent; }
	inline ShapeComponentPtr getShapeComponent() const { return m_shapeComponent; }

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "Shape"; }
	virtual const ImVec4 editorGetIconColor() const override;

protected:
	ShapeComponentPtr m_shapeComponent;
	eShapeType m_shapeType = eShapeType::INVALID;
};

using GraphShapePropertyFactory = TypedGraphPropertyFactory<GraphShapeProperty, GraphShapePropertyConfig>;
