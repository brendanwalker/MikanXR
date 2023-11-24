#pragma once

#include "NodeFwd.h"

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

	virtual void onEditorContextMenuUI(class NodeEditorState* editorState) {}

protected:
	int m_id;
	int m_size;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_parentNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
};