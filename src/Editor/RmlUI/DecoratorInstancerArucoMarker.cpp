#include "DecoratorInstancerArucoMarker.h"
#include "DecoratorArucoMarker.h"

#include <RmlUi/Core/PropertyDefinition.h>

DecoratorInstancerArucoMarker::DecoratorInstancerArucoMarker()
{
	// Register CSS properties for ArUco marker configuration
	m_idDictionaryType = RegisterProperty("dictionary-type", "0").AddParser("number").GetId();
	m_idArucoId = RegisterProperty("aruco-id", "0").AddParser("number").GetId();
	m_idArucoSize = RegisterProperty("aruco-size", "200").AddParser("number").GetId();

	// Register shorthand to allow using properties in decorator declaration
	// Using RecursiveCommaSeparated to properly parse comma-separated parameters
	RegisterShorthand("decorator", "dictionary-type, aruco-id, aruco-size", Rml::ShorthandType::RecursiveCommaSeparated);
}

DecoratorInstancerArucoMarker::~DecoratorInstancerArucoMarker()
{
}

Rml::SharedPtr<Rml::Decorator> DecoratorInstancerArucoMarker::InstanceDecorator(
	const Rml::String& name,
	const Rml::PropertyDictionary& properties,
	const Rml::DecoratorInstancerInterface& instancer_interface)
{
	// Get ArUco properties
	int dictionary_type = properties.GetProperty(m_idDictionaryType)->Get<int>();
	int aruco_id = properties.GetProperty(m_idArucoId)->Get<int>();
	int aruco_size = properties.GetProperty(m_idArucoSize)->Get<int>();

	auto decorator = Rml::MakeShared<DecoratorArucoMarker>();

	if (decorator->Initialise(dictionary_type, aruco_id, aruco_size))
	{
		return decorator;
	}

	return nullptr;
}