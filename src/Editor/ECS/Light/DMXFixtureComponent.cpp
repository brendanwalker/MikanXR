#include "DMXFixtureComponent.h"
#include "MikanLightTypes.h"
#include "MikanObject.h"
#include "MikanVariantTypes.h"
#include "StringUtils.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- DMXFixtureComponentDefinition -----
const std::string DMXFixtureComponentDefinition::k_dmxUniversePropertyId = "dmx_universe";
const std::string DMXFixtureComponentDefinition::k_dmxStartChannelPropertyId = "dmx_start_channel";
const std::string DMXFixtureComponentDefinition::k_dmxChannelCountPropertyId = "dmx_channel_count";
const std::string DMXFixtureComponentDefinition::k_isDisabledPropertyId = "is_disabled";

DMXFixtureComponentDefinition::DMXFixtureComponentDefinition()
	: TransformComponentDefinition()
{
}

DMXFixtureComponentDefinition::DMXFixtureComponentDefinition(MikanLightID lightId)
	: TransformComponentDefinition(lightId)
{
}

configuru::Config DMXFixtureComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_dmxUniversePropertyId] = m_dmxUniverse;
	pt[k_dmxStartChannelPropertyId] = m_dmxStartChannel;
	pt[k_dmxChannelCountPropertyId] = m_dmxChannelCount;
	pt[k_isDisabledPropertyId] = m_bIsDisabled;

	return pt;
}

void DMXFixtureComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_dmxUniverse = pt.get_or<uint16_t>(k_dmxUniversePropertyId, 1);
	m_dmxStartChannel = pt.get_or<uint16_t>(k_dmxStartChannelPropertyId, 1);
	m_dmxChannelCount = pt.get_or<uint16_t>(k_dmxChannelCountPropertyId, 3);
	m_bIsDisabled = pt.get_or<bool>(k_isDisabledPropertyId, false);
}

bool DMXFixtureComponentDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* values = initParams.getTypedPointer<MikanDMXFixtureComponentValues>();
	if (values)
	{
		m_dmxUniverse = values->dmx_universe;
		m_dmxStartChannel = values->dmx_start_channel;
		m_dmxChannelCount = values->dmx_channel_count;
		m_bIsDisabled = values->is_disabled;
	}

	return true;
}

void DMXFixtureComponentDefinition::setDMXUniverse(uint16_t universe)
{
	if (m_dmxUniverse != universe)
	{
		m_dmxUniverse = universe;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_dmxUniversePropertyId));
	}
}

void DMXFixtureComponentDefinition::setDMXStartChannel(uint16_t startChannel)
{
	if (m_dmxStartChannel != startChannel)
	{
		m_dmxStartChannel = startChannel;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_dmxStartChannelPropertyId));
	}
}

void DMXFixtureComponentDefinition::setIsDisabled(bool flag)
{
	if (m_bIsDisabled != flag)
	{
		m_bIsDisabled = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_isDisabledPropertyId));
	}
}

void DMXFixtureComponentDefinition::setDMXChannelCount(uint16_t count)
{
	if (m_dmxChannelCount != count)
	{
		m_dmxChannelCount = count;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_dmxChannelCountPropertyId));
	}
}

// -- DMXFixtureComponent -----
DMXFixtureComponent::DMXFixtureComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
}

// -- IEntityAccessor --
rfk::Struct const* DMXFixtureComponent::getClientAPIValuesStructType() const
{
	return &MikanDMXFixtureComponentValues::staticGetArchetype();
}

// -- IPropertyInterface --
void DMXFixtureComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			DMXFixtureComponentDefinition::k_dmxUniversePropertyId, MikanVariantType::INT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			DMXFixtureComponentDefinition::k_dmxStartChannelPropertyId, MikanVariantType::INT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			DMXFixtureComponentDefinition::k_dmxChannelCountPropertyId, MikanVariantType::INT)
		->setReadOnly());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			DMXFixtureComponentDefinition::k_isDisabledPropertyId, MikanVariantType::BOOL));
}

bool DMXFixtureComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	DMXFixtureComponentDefinitionPtr def = getDMXFixtureDefinition();

	if (propertyName == DMXFixtureComponentDefinition::k_dmxUniversePropertyId)
	{
		outValue = static_cast<int>(def->getDMXUniverse());
		return true;
	}
	else if (propertyName == DMXFixtureComponentDefinition::k_dmxStartChannelPropertyId)
	{
		outValue = static_cast<int>(def->getDMXStartChannel());
		return true;
	}
	else if (propertyName == DMXFixtureComponentDefinition::k_dmxChannelCountPropertyId)
	{
		outValue = static_cast<int>(def->getDMXChannelCount());
		return true;
	}
	else if (propertyName == DMXFixtureComponentDefinition::k_isDisabledPropertyId)
	{
		outValue = def->getIsDisabled();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool DMXFixtureComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	DMXFixtureComponentDefinitionPtr def = getDMXFixtureDefinition();

	if (propertyName == DMXFixtureComponentDefinition::k_dmxUniversePropertyId)
	{
		def->setDMXUniverse(static_cast<uint16_t>(inValue.getIntValue()));
		return true;
	}
	else if (propertyName == DMXFixtureComponentDefinition::k_dmxStartChannelPropertyId)
	{
		def->setDMXStartChannel(static_cast<uint16_t>(inValue.getIntValue()));
		return true;
	}
	else if (propertyName == DMXFixtureComponentDefinition::k_isDisabledPropertyId)
	{
		def->setIsDisabled(inValue.getBoolValue());
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- Lua Binding --
void DMXFixtureComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<DMXFixtureComponent, TransformComponent>(
			DMXFixtureComponent::k_componentClassName.c_str())
		.addProperty("dmxUniverse",
			[](DMXFixtureComponent* c) -> int {
				return static_cast<int>(c->getDMXFixtureDefinition()->getDMXUniverse());
			},
			[](DMXFixtureComponent* c, int v) {
				c->getDMXFixtureDefinition()->setDMXUniverse(static_cast<uint16_t>(v));
			})
		.addProperty("dmxStartChannel",
			[](DMXFixtureComponent* c) -> int {
				return static_cast<int>(c->getDMXFixtureDefinition()->getDMXStartChannel());
			},
			[](DMXFixtureComponent* c, int v) {
				c->getDMXFixtureDefinition()->setDMXStartChannel(static_cast<uint16_t>(v));
			})
		.addProperty("isDisabled",
			[](DMXFixtureComponent* c) -> bool {
				return c->getDMXFixtureDefinition()->getIsDisabled();
			},
			[](DMXFixtureComponent* c, bool v) {
				c->getDMXFixtureDefinition()->setIsDisabled(v);
			})
		.endClass();
}
