#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkGuiScopedDragDropTarget
{
public:
	MkGuiScopedDragDropTarget();
	~MkGuiScopedDragDropTarget();

	MkGuiScopedDragDropTarget(const MkGuiScopedDragDropTarget&)= delete;
	MkGuiScopedDragDropTarget& operator=(const MkGuiScopedDragDropTarget&)= delete;

	explicit operator bool() const { return m_active; }

private:
	bool m_active= false;
};
