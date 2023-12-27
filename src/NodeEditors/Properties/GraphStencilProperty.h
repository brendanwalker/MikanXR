#pragma once

#include "AssetFwd.h"
#include "ComponentFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"
#include "ProfileConfigConstants.h"

class GraphStencilPropertyConfig : public GraphPropertyConfig
{
public:
	GraphStencilPropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eStencilType stencilType;
	std::string stencilName;
};

class GraphStencilProperty : public GraphProperty
{
public:
	GraphStencilProperty() = default;

	inline static const std::string k_propertyClassName = "GraphStencilProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	inline void setStencilComponent(StencilComponentPtr inComponent) { m_stencilComponent = inComponent; }
	inline StencilComponentPtr getStencilComponent() const { return m_stencilComponent; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	StencilComponentPtr m_stencilComponent;
	eStencilType m_stencilType;
};

using GraphStencilPropertyFactory = TypedGraphPropertyFactory<GraphStencilProperty, GraphStencilPropertyConfig>;