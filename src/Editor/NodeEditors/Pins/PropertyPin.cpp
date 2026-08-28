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

MkCanvas::PinIcon PropertyPin::editorGetPinIcon() const { return MkCanvas::PinIcon::Circle; }

ImVec4 PropertyPin::editorGetPinColor() const
{
	// Untyped property pins render darker than ones bound to a property class
	return !m_propertyClassName.empty() ? ImVec4(148.f / 255.f, 0.f, 0.f, 1.f) : ImVec4(70.f / 255.f, 0.f, 0.f, 1.f);
}

void PropertyPin::editorRenderContextMenu(const NodeEditorState& editorState)
{
	// TODO: If we are an array input pin, offer a deletion option
}

ImU32 PropertyPin::editorGetLinkStyleColor() const { return IM_COL32(148, 0, 0, 255); }