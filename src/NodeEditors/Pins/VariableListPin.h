#pragma once

#include "NodePin.h"
#include <vector>

class VariableListPinConfig : public NodePinConfig
{
public:
	VariableListPinConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string variableClassName;
};


class VariableListPin : public NodePin
{
public:
	VariableListPin() = default;

	virtual bool loadFromConfig(NodePinConfigConstPtr config) override;
	virtual void saveToConfig(NodePinConfigPtr config) const override;

	inline void setVariableClassName(const std::string& varClassName) { m_variableClassName= varClassName; }
	inline const std::string& getVariableClassName() const { return m_variableClassName; }

	inline const GraphVariableListPtr getValue() const { return m_variableList; }
	inline void setValue(GraphVariableListPtr inArray) { m_variableList = inArray; }

	inline static const std::string k_nodeClassName = "VariableListPin";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual size_t getDataSize() const { return sizeof(GraphVariableListPtr); }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const override;
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	GraphVariableListPtr m_variableList;
	std::string m_variableClassName;
};