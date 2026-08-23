#include "PropertyPin.h"
#include "Properties/GraphProperty.h"

// -- PropertyPinConfig -----
configuru::Config PropertyPinConfig::writeToJSON()
{
	configuru::Config pt= NodePinConfig::writeToJSON();

	pt["property_class_name"]= propertyClassName;

	return pt;
}

void PropertyPinConfig::readFromJSON(const configuru::Config& pt)
{
	propertyClassName= pt.get_or<std::string>("property_class_name", "");

	NodePinConfig::readFromJSON(pt);
}

// -- PropertyPin --
bool PropertyPin::loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config)
{
	if (!NodePin::loadFromConfig(ownerGraph, config))
		return false;

	auto propertyPinConfig= std::static_pointer_cast<const PropertyPinConfig>(config);
	m_propertyClassName= propertyPinConfig->propertyClassName;

	return true;
}

void PropertyPin::saveToConfig(NodePinConfigPtr config) const
{
	auto propertyPinConfig= std::static_pointer_cast<PropertyPinConfig>(config);

	propertyPinConfig->propertyClassName= m_propertyClassName;
	NodePin::saveToConfig(config);
}

bool PropertyPin::canPinsBeConnected(NodePinPtr otherPinPtr) const
{
	if (!NodePin::canPinsBeConnected(otherPinPtr))
		return false;

	// Only connect variable list of the same property type
	auto otherPropertyPin= std::static_pointer_cast<PropertyPin>(otherPinPtr);
	if (this->getPropertyClassName() != otherPropertyPin->getPropertyClassName())
		return false;

	return true;
}

void PropertyPin::copyValueFromSourcePin()
{
	auto sourcePin= std::dynamic_pointer_cast<PropertyPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

ImNodesPinShape PropertyPin::editorComputePinShape() const
{
	if (m_connectedLinks.size() > 0)
		return ImNodesPinShape_CircleFilled;
	else
		return ImNodesPinShape_Circle;
}

std::shared_ptr<MkNodesScopedColorStyle> PropertyPin::editorRenderMakePinStyle(float alpha)
{
	const unsigned int pinColor= !m_propertyClassName.empty() ? IM_COL32(148, 0, 0, (unsigned char)(alpha * 255))
															  : IM_COL32(70, 0, 0, (unsigned char)(alpha * 255));
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Pin, pinColor)
		.push(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, (unsigned char)(alpha * 255)));
	return style;
}

std::shared_ptr<MkNodesScopedColorStyle> PropertyPin::editorRenderMakeLinkStyle(float alpha)
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Link, IM_COL32(148, 0, 0, (unsigned char)alpha))
		.push(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, (unsigned char)alpha))
		.push(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
	return style;
}

void PropertyPin::editorRenderContextMenu(const NodeEditorState& editorState)
{
	// TODO: If we are an array input pin, offer a deletion option
}

ImU32 PropertyPin::editorGetLinkStyleColor() const { return IM_COL32(148, 0, 0, 255); }