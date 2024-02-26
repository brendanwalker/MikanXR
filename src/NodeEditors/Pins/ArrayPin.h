#pragma once

#include "NodePin.h"
#include <vector>

class ArrayPinConfig : public NodePinConfig
{
public:
	ArrayPinConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string elementClassName;
};

class ArrayPin : public NodePin
{
public:
	ArrayPin() = default;

	virtual bool loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config) override;
	virtual void saveToConfig(NodePinConfigPtr config) const override;

	inline void setElementClassName(const std::string& varClassName) { m_elementClassName= varClassName; }
	inline const std::string& getElementClassName() const { return m_elementClassName; }

	inline const std::vector<GraphPropertyPtr>& getArray() const { return m_array; }
	inline void setArray(const std::vector<GraphPropertyPtr>& inArray) { m_array = inArray; }
	inline void clearArray() { m_array.clear(); }

	inline static const std::string k_pinClassName = "ArrayPin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual size_t getDataSize() const { return sizeof(GraphPropertyPtr) * m_array.size(); }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const override;
	virtual void copyValueFromSourcePin() override;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	std::vector<GraphPropertyPtr> m_array;
	std::string m_elementClassName;
};