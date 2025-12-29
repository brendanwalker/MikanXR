#pragma once

#include <RmlUi/Core/DecoratorInstancer.h>

class DecoratorInstancerArucoMarker : public Rml::DecoratorInstancer
{
public:
	DecoratorInstancerArucoMarker();
	virtual ~DecoratorInstancerArucoMarker();

	/// Instances a decorator given the property tag and attributes from the RCSS file.
	/// @param name[in] The type of decorator desired.
	/// @param properties[in] All RCSS properties associated with the decorator.
	/// @param instancer_interface[in] An interface for querying the active style sheet.
	/// @return A shared_ptr to the decorator if it was instanced successfully.
	Rml::SharedPtr<Rml::Decorator> InstanceDecorator(
		const Rml::String& name,
		const Rml::PropertyDictionary& properties,
		const Rml::DecoratorInstancerInterface& instancer_interface) override;

private:
	Rml::PropertyId m_idDictionaryType;
	Rml::PropertyId m_idArucoId;
	Rml::PropertyId m_idArucoSize;
};