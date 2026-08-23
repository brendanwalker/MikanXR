#include "MkNodesScopedNodeEditor.h"

#include "imnodes.h"

MkNodesScopedNodeEditor::MkNodesScopedNodeEditor() { ImNodes::BeginNodeEditor(); }

MkNodesScopedNodeEditor::~MkNodesScopedNodeEditor() { ImNodes::EndNodeEditor(); }
