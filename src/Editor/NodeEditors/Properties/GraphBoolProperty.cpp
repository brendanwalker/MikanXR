#include "GraphBoolProperty.h"
#include "NodeEditorUI.h"

void GraphBoolProperty::editorRenderValue(const NodeEditorState& editorState)
{
	NodeEditorUI::DrawCheckBoxProperty("boolPropertyDefaultValue", "Default", m_value);
	NodeEditorUI::DrawStaticTextProperty("Value", m_value ? "True" : "False");
}