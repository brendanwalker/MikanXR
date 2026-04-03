#pragma once

class MkNodesScopedNode
{
public:
	explicit MkNodesScopedNode(int id);
	~MkNodesScopedNode();

	MkNodesScopedNode(const MkNodesScopedNode&) = delete;
	MkNodesScopedNode& operator=(const MkNodesScopedNode&) = delete;
};
