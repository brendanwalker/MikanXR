#include "PropertyPin.h"
#include "Properties/GraphProperty.h"

// -- PropertyPinConfig -----
configuru::Config PropertyPinConfig::writeToJSON()
{
	configuru::Config pt = NodePinConfig::writeToJSON();

	pt["property_class_name"] = propertyClassName;

	return pt;
}

void PropertyPinConfig::readFromJSON(const configuru::Config& pt)
{
	propertyClassName = pt.get_or<std::string>("property_class_name", "");

	NodePinConfig::readFromJSON(pt);
}

// -- PropertyPin --
bool PropertyPin::loadFromConfig(NodePinConfigConstPtr config)
{
	if (!NodePin::loadFromConfig(config))
		return false;

	auto propertyPinConfig = std::static_pointer_cast<const PropertyPinConfig>(config);
	m_propertyClassName = propertyPinConfig->propertyClassName;

	return true;
}

void PropertyPin::saveToConfig(NodePinConfigPtr config) const
{
	auto propertyPinConfig = std::static_pointer_cast<PropertyPinConfig>(config);

	propertyPinConfig->propertyClassName = m_propertyClassName;
	NodePin::saveToConfig(config);
}

bool PropertyPin::canPinsBeConnected(NodePinPtr otherPinPtr) const
{
	if (!NodePin::canPinsBeConnected(otherPinPtr))
		return false;

	// Only connect variable list of the same property type
	auto otherPropertyPin = std::static_pointer_cast<PropertyPin>(otherPinPtr);
	if (this->getPropertyClassName() != otherPropertyPin->getPropertyClassName())
		return false;

	return true;
}

void PropertyPin::copyValueFromSourcePin()
{
	auto sourcePin = std::dynamic_pointer_cast<PropertyPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

ImNodesPinShape PropertyPin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	// Darken pin color when there is no property class assigned
	if (!m_propertyClassName.empty())
		ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(148, 0, 0, alpha * 255));
	else
		ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(70, 0, 0, alpha * 255));

	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, alpha * 255));

	return pinShape;
}

void PropertyPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(148, 0, 0, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
}

void PropertyPin::editorRenderContextMenu(const NodeEditorState& editorState)
{
	//TODO: If we are an array input pin, offer a deletion option
}

ImU32 PropertyPin::editorGetLinkStyleColor() const
{
	return IM_COL32(148, 0, 0, 255);
}