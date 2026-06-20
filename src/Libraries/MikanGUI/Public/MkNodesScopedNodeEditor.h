#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkNodesScopedNodeEditor
{
public:
	MkNodesScopedNodeEditor();
	~MkNodesScopedNodeEditor();

	MkNodesScopedNodeEditor(const MkNodesScopedNodeEditor&)= delete;
	MkNodesScopedNodeEditor& operator=(const MkNodesScopedNodeEditor&)= delete;
};
