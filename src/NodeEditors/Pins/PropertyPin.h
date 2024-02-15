#pragma once

#include "RendererFwd.h"
#include "NodePin.h"

class PropertyPinConfig : public NodePinConfig
{
public:
	PropertyPinConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string propertyClassName;
};

class PropertyPin : public NodePin
{
public:
	PropertyPin() = default;

	virtual bool loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config) override;
	virtual void saveToConfig(NodePinConfigPtr config) const override;

	inline void setPropertyClassName(const std::string& varClassName) { m_propertyClassName = varClassName; }
	inline const std::string& getPropertyClassName() const { return m_propertyClassName; }

	GraphPropertyPtr getValue() const { return m_propterty; }
	void setValue(GraphPropertyPtr inProperty) { m_propterty = inProperty; }

	inline static const std::string k_pinClassName = "PropertyPin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual size_t getDataSize() const { return sizeof(GraphPropertyPtr); }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const override;
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	GraphPropertyPtr m_propterty;
	std::string m_propertyClassName;
};