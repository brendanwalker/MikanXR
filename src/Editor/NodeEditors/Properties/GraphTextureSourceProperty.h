#pragma once

#include "AssetFwd.h"
#include "ComponentFwd.h"
#include "MikanRendererFwd.h"
#include "GraphProperty.h"
#include "ProjectConfigConstants.h"

class GraphTextureSourcePropertyConfig : public GraphPropertyConfig
{
public:
	GraphTextureSourcePropertyConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eTextureSourceType textureSourceType;
	std::string textureSourceName;
};

class GraphTextureSourceProperty : public GraphProperty
{
public:
	GraphTextureSourceProperty()= default;

	inline static const std::string k_propertyClassName= "GraphTextureSourceProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig, const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	inline void setTextureSourceComponent(TextureSourceComponentPtr inComponent)
	{
		m_textureSourceComponent= inComponent;
	}
	inline TextureSourceComponentPtr getTextureSourceComponent() const { return m_textureSourceComponent; }

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "TextureSource"; }
	virtual const ImVec4 editorGetIconColor() const override;

protected:
	TextureSourceComponentPtr m_textureSourceComponent;
	eTextureSourceType m_textureSourceType;
};

using GraphTextureSourcePropertyFactory=
	TypedGraphPropertyFactory<GraphTextureSourceProperty, GraphTextureSourcePropertyConfig>;