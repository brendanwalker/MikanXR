#include "MkNodesScopedColorStyle.h"

#include "imnodes.h"

MkNodesScopedColorStyle::~MkNodesScopedColorStyle()
{
	for (int i = 0; i < m_count; ++i)
		ImNodes::PopColorStyle();
}

MkNodesScopedColorStyle& MkNodesScopedColorStyle::push(int item, unsigned int color)
{
	ImNodes::PushColorStyle(static_cast<ImNodesCol>(item), color);
	++m_count;
	return *this;
}
