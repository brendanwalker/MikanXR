#pragma once

class MkNodesScopedNodeEditor
{
public:
	MkNodesScopedNodeEditor();
	~MkNodesScopedNodeEditor();

	MkNodesScopedNodeEditor(const MkNodesScopedNodeEditor&) = delete;
	MkNodesScopedNodeEditor& operator=(const MkNodesScopedNodeEditor&) = delete;
};
