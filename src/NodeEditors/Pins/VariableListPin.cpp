#include "VariableListPin.h"

// -- VariableListPinConfig -----
configuru::Config VariableListPinConfig::writeToJSON()
{
	configuru::Config pt = NodePinConfig::writeToJSON();

	pt["variable_class_name"] = variableClassName;

	return pt;
}

void VariableListPinConfig::readFromJSON(const configuru::Config& pt)
{
	variableClassName= pt.get_or<std::string>(variableClassName);

	NodePinConfig::readFromJSON(pt);
}

// -- VariableListPin -----
bool VariableListPin::loadFromConfig(NodePinConfigConstPtr config)
{
	if (!NodePin::loadFromConfig(config))
		return false;

	auto varListPinConfig= std::static_pointer_cast<const VariableListPinConfig>(config);
	m_variableClassName= varListPinConfig->variableClassName;

	return true;
}

void VariableListPin::saveToConfig(NodePinConfigPtr config) const
{
	auto varListPinConfig = std::static_pointer_cast<VariableListPinConfig>(config);

	varListPinConfig->variableClassName= m_variableClassName;
	NodePin::saveToConfig(config);
}

bool VariableListPin::canPinsBeConnected(NodePinPtr otherPinPtr) const
{
	if (!NodePin::canPinsBeConnected(otherPinPtr))
	{
		return false;
	}

	// Only connect variable list of the same element type
	auto otherVarListPin = std::static_pointer_cast<VariableListPin>(otherPinPtr);
	if (this->m_variableClassName != otherVarListPin->getVariableClassName())
	{
		return false;
	}

	return true;
}

void VariableListPin::copyValueFromSourcePin()
{
	auto sourcePin = std::dynamic_pointer_cast<VariableListPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

ImNodesPinShape VariableListPin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_QuadFilled;
	else
		pinShape = ImNodesPinShape_Quad;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(148, 0, 0, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, alpha * 255));

	return pinShape;
}

void VariableListPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(148, 0, 0, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
}

void VariableListPin::editorRenderContextMenu(const NodeEditorState& editorState)
{
}

ImU32 VariableListPin::editorGetLinkStyleColor() const
{
	return IM_COL32(148, 0, 0, 255);
}
