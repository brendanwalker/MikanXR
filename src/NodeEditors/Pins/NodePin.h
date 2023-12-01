#pragma once

#include "NodeFwd.h"

#include "imnodes.h"

#include <string>
#include <vector>

enum class eNodePinDirection : int
{
	INPUT,
	OUTPUT
};

class NodePin
{
public:
	NodePin();
	NodePin(NodePtr ownerNode);

	inline int getId() const { return m_id; }
	inline eNodePinDirection getDirection() const { return m_direction; }
	inline const std::string& getName() const { return m_name; }
	inline NodePtr getOwnerNode() const { return m_ownerNode; }
	inline const std::vector<NodeLinkPtr>& getConnectedLinks() const { return m_connectedLinks; }

	virtual size_t getDataSize() const { return 0; }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const;

	virtual float editorComputeInputWidth() const;
	virtual float editorComputeNodeAlpha(class NodeEditorState* editorState) const;
	virtual void editorRenderInputPin(class NodeEditorState* editorState);
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) {}
	virtual void editorRenderOutputPin(class NodeEditorState* editorState, float prefixWidth= 0.f);
	virtual ImNodesPinShape editorRenderBeginPin(float alpha);
	virtual void editorRenderEndPin();
	virtual void editorRenderBeginLink(float alpha);
	virtual void editorRenderEndLink();
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) {}
	virtual ImU32 editorGetLinkStyleColor() const;

protected:
	int m_id;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_ownerNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
};