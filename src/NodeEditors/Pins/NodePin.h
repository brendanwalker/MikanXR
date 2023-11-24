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
	inline int getSize() const { return m_size; }
	inline eNodePinDirection getDirection() const { return m_direction; }
	inline const std::string& getName() const { return m_name; }
	inline NodePtr getOwnerNode() const { return m_ownerNode; }
	inline const std::vector<NodeLinkPtr>& getConnectedLinks() const { return m_connectedLinks; }

	virtual ImNodesPinShape editorRenderBeginPin(float alpha);
	virtual void editorRenderEndPin();
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) {}

protected:
	int m_id;
	int m_size;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_ownerNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
};