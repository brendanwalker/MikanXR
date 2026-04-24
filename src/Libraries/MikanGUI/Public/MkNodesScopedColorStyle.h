#pragma once

#include "MkGuiExport.h"

class MIKAN_GUI_CLASS MkNodesScopedColorStyle
{
public:
	MkNodesScopedColorStyle() = default;
	~MkNodesScopedColorStyle();

	MkNodesScopedColorStyle(const MkNodesScopedColorStyle&) = delete;
	MkNodesScopedColorStyle& operator=(const MkNodesScopedColorStyle&) = delete;

	// ImNodesCol is typedef int; color is ImU32 (typedef unsigned int)
	MkNodesScopedColorStyle& push(int item, unsigned int color);

private:
	int m_count = 0;
};
