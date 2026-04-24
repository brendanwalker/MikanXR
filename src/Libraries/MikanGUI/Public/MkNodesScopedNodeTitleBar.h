#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkNodesScopedNodeTitleBar
{
public:
	MkNodesScopedNodeTitleBar();
	~MkNodesScopedNodeTitleBar();

	MkNodesScopedNodeTitleBar(const MkNodesScopedNodeTitleBar&) = delete;
	MkNodesScopedNodeTitleBar& operator=(const MkNodesScopedNodeTitleBar&) = delete;
};
