#pragma once

#include "NodeFwd.h"
#include "NodePinConstants.h"
#include "MulticastDelegate.h"

#include "imnodes.h"

#include <string>
#include <vector>

class NodePin
{
public:
	NodePin();
	NodePin(NodePtr ownerNode);

	inline t_node_pin_id getId() const { return m_id; }
	inline eNodePinDirection getDirection() const { return m_direction; }
	inline const std::string& getName() const { return m_name; }
	inline NodePtr getOwnerNode() const { return m_ownerNode; }
	inline const std::vector<NodeLinkPtr>& getConnectedLinks() const { return m_connectedLinks; }

	inline void setName(const std::string& name) { m_name= name; }
	inline void setDirection(eNodePinDirection direction) { m_direction= direction; }

	virtual bool connectLink(NodeLinkPtr linkPtr);
	MulticastDelegate<void(t_node_link_id id)> OnLinkConnected;
	virtual bool disconnectLink(NodeLinkPtr linkPtr);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDisconnected;

	virtual size_t getDataSize() const { return 0; }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const;

	NodePinPtr getConnectedSourcePin() const;
	virtual void copyValueFromSourcePin() {}

	virtual float editorComputeInputWidth() const;
	virtual float editorComputeNodeAlpha(const NodeEditorState& editorState) const;
	virtual void editorRenderInputPin(const NodeEditorState& editorState);
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) {}
	virtual void editorRenderOutputPin(const NodeEditorState& editorState, float prefixWidth= 0.f);
	virtual ImNodesPinShape editorRenderBeginPin(float alpha);
	virtual void editorRenderEndPin();
	virtual void editorRenderBeginLink(float alpha);
	virtual void editorRenderEndLink();
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) {}
	virtual ImU32 editorGetLinkStyleColor() const;

protected:
	t_node_pin_id m_id;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_ownerNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
};