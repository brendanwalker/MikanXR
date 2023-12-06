#pragma once
#include "Node.h"

class EventNode : public Node
{
public:
	EventNode();
	EventNode(NodeGraphPtr parentGraph);

	inline void setName(const std::string& inName) { m_eventName= inName; }
	inline const std::string& getName() const { return m_eventName; }

	virtual bool editorCanDelete() const override { return false; }

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	std::string m_eventName;
};