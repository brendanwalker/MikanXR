#include "GraphBoolProperty.h"
#include "NodeEditorState.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"

void GraphBoolProperty::editorRenderValue(const NodeEditorState& editorState)
{
	MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

	MkGui::drawCheckBoxProperty(propertyStyle, "boolPropertyDefaultValue", locText("graphProperties.default"), m_value);
	MkGui::drawStaticTextProperty(propertyStyle, locText("graphProperties.value"), m_value ? "True" : "False");
}