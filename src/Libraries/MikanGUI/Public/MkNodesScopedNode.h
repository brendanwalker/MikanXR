#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkNodesScopedNode
{
public:
	explicit MkNodesScopedNode(int id);
	~MkNodesScopedNode();

	MkNodesScopedNode(const MkNodesScopedNode&) = delete;
	MkNodesScopedNode& operator=(const MkNodesScopedNode&) = delete;
};
