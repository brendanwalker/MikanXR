#include "MkNodesScopedNodeTitleBar.h"

#include "imnodes.h"

MkNodesScopedNodeTitleBar::MkNodesScopedNodeTitleBar()
{
	ImNodes::BeginNodeTitleBar();
}

MkNodesScopedNodeTitleBar::~MkNodesScopedNodeTitleBar()
{
	ImNodes::EndNodeTitleBar();
}
