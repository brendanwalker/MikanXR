#include "ArrayPin.h"

// -- ArrayPinConfig -----
configuru::Config ArrayPinConfig::writeToJSON()
{
	configuru::Config pt= NodePinConfig::writeToJSON();

	pt["element_class_name"]= elementClassName;

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
	auto varListPinConfig= std::static_pointer_cast<ArrayPinConfig>(config);

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
	auto otherVarListPin= std::static_pointer_cast<ArrayPin>(otherPinPtr);
	if (this->m_elementClassName != otherVarListPin->getElementClassName())
	{
		return false;
	}

	return true;
}

void ArrayPin::copyValueFromSourcePin()
{
	auto sourcePin= std::dynamic_pointer_cast<ArrayPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setArray(sourcePin->getArray());
	}
}

MkCanvas::PinIcon ArrayPin::editorGetPinIcon() const { return MkCanvas::PinIcon::Square; }

ImVec4 ArrayPin::editorGetPinColor() const { return ImVec4(148.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.f); }

void ArrayPin::editorRenderContextMenu(const NodeEditorState& editorState) {}

ImU32 ArrayPin::editorGetLinkStyleColor() const { return IM_COL32(148, 0, 0, 255); }
