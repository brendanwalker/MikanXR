#include "ArrayPin.h"

// -- ArrayPinConfig -----
configuru::Config ArrayPinConfig::writeToJSON()
{
	configuru::Config pt = NodePinConfig::writeToJSON();

	pt["element_class_name"] = elementClassName;

	return pt;
}

void ArrayPinConfig::readFromJSON(const configuru::Config& pt)
{
	elementClassName= pt.get_or<std::string>("element_class_name", "");

	NodePinConfig::readFromJSON(pt);
}

// -- ArrayPin -----
bool ArrayPin::loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config)
{
	if (!NodePin::loadFromConfig(ownerGraph, config))
		return false;

	auto varListPinConfig= std::static_pointer_cast<const ArrayPinConfig>(config);
	m_elementClassName= varListPinConfig->elementClassName;

	return true;
}

void ArrayPin::saveToConfig(NodePinConfigPtr config) const
{
	auto varListPinConfig = std::static_pointer_cast<ArrayPinConfig>(config);

	varListPinConfig->elementClassName= m_elementClassName;
	NodePin::saveToConfig(config);
}

bool ArrayPin::canPinsBeConnected(NodePinPtr otherPinPtr) const
{
	if (!otherPinPtr)
		return false;

	// Are we trying to connect a pin back to itself?
	if (otherPinPtr.get() == this)
		return false;

	// Are pins not of the same type?
	if (this->getClassName() != otherPinPtr->getClassName())
		return false;

	// Is one pin an input and the other an output?
	if (this->getDirection() == otherPinPtr->getDirection())
		return false;

	// Only connect variable list of the same element type
	auto otherVarListPin = std::static_pointer_cast<ArrayPin>(otherPinPtr);
	if (this->m_elementClassName != otherVarListPin->getElementClassName())
	{
		return false;
	}

	return true;
}

void ArrayPin::copyValueFromSourcePin()
{
	auto sourcePin = std::dynamic_pointer_cast<ArrayPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setArray(sourcePin->getArray());
	}
}

ImNodesPinShape ArrayPin::editorRenderBeginPin(float alpha)
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

void ArrayPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(148, 0, 0, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
}

void ArrayPin::editorRenderContextMenu(const NodeEditorState& editorState)
{
}

ImU32 ArrayPin::editorGetLinkStyleColor() const
{
	return IM_COL32(148, 0, 0, 255);
}
