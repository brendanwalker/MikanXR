#include "DMXObjectSystem.h"
#include "MikanPropertyDatabase.h"
#include "MikanFunctionDatabase.h"
#include "ProjectManager.h"
#include "MikanLightTypes.h"
#include "ProjectConfig.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- DMXObjectSystemDefinition -----
const std::string DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId= "network_interface_ip";
const std::string DMXObjectSystemDefinition::k_dmxPriorityPropertyId= "dmx_priority";
const std::string DMXObjectSystemDefinition::k_transmitRateHzPropertyId= "transmit_rate_hz";

configuru::Config DMXObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt= MikanObjectSystemDefinition::writeToJSON();

	pt[k_networkInterfaceIPPropertyId]= m_dmxConfig.networkInterfaceIP;
	pt[k_dmxPriorityPropertyId]= m_dmxConfig.priority;
	pt[k_transmitRateHzPropertyId]= m_dmxConfig.transmitRateHz;

	return pt;
}

void DMXObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_dmxConfig.networkInterfaceIP= pt.get_or<std::string>(k_networkInterfaceIPPropertyId, "0.0.0.0");
	m_dmxConfig.priority= static_cast<uint8_t>(pt.get_or<int>(k_dmxPriorityPropertyId, 100));
	m_dmxConfig.transmitRateHz= pt.get_or<float>(k_transmitRateHzPropertyId, 44.0f);
}

void DMXObjectSystemDefinition::setNetworkInterfaceIP(const std::string& ip)
{
	if (m_dmxConfig.networkInterfaceIP != ip)
	{
		m_dmxConfig.networkInterfaceIP= ip;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_networkInterfaceIPPropertyId));
	}
}

void DMXObjectSystemDefinition::setDMXPriority(uint8_t priority)
{
	if (m_dmxConfig.priority != priority)
	{
		m_dmxConfig.priority= priority;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_dmxPriorityPropertyId));
	}
}

void DMXObjectSystemDefinition::setTransmitRateHz(float hz)
{
	if (m_dmxConfig.transmitRateHz != hz)
	{
		m_dmxConfig.transmitRateHz= hz;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_transmitRateHzPropertyId));
	}
}

// -- DMXObjectSystem -----
bool DMXObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	// Start the DMX manager
	m_dmxManager= IDMXManager::create();
	if (!m_dmxManager->startup(getDMXManagerConfig()))
	{
		// Non-fatal: editor still loads, fixtures just won't send DMX
		m_dmxManager.reset();
	}

	// Listen for config property changes so we can restart the DMX manager
	definitionPtr->OnPropertyChanged+= MakeDelegate(this, &DMXObjectSystem::onDefinitionMarkedDirty);

	return true;
}

void DMXObjectSystem::dispose()
{
	if (m_dmxManager)
	{
		m_dmxManager->shutdown();
		m_dmxManager.reset();
	}

	auto definitionPtr= getDefinition();
	if (definitionPtr)
	{
		definitionPtr->OnPropertyChanged-= MakeDelegate(this, &DMXObjectSystem::onDefinitionMarkedDirty);
	}

	MikanObjectSystem::dispose();
}

DMXObjectSystemDefinitionConstPtr DMXObjectSystem::getDMXObjectSystemConfigConst() const
{
	auto projectConfig= getProjectConfig();

	return projectConfig ? projectConfig->dmxObjectSystemDefinition : DMXObjectSystemDefinitionConstPtr();
}

DMXObjectSystemDefinitionPtr DMXObjectSystem::getDMXObjectSystemConfig()
{
	return std::const_pointer_cast<DMXObjectSystemDefinition>(getDMXObjectSystemConfigConst());
}

MikanComponentPtr DMXObjectSystem::getComponentById(int componentId) const
{
	// DMXObjectSystem doesn't manage components by ID
	return MikanComponentPtr();
}

bool DMXObjectSystem::getComponentList(const std::string& componentClassName,
									   std::vector<MikanComponentPtr>& outComponentList) const
{
	// DMXObjectSystem doesn't manage ownership of components
	return false;
}

bool DMXObjectSystem::getComponentIdList(const std::string& componentClassName,
										 std::vector<int>& outComponentIdList) const
{
	// DMXObjectSystem doesn't manage ownership of components
	return false;
}

// Project Config Events
void DMXObjectSystem::onDefinitionMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changeSet)
{
	if (changeSet.hasPropertyName(DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId)
		|| changeSet.hasPropertyName(DMXObjectSystemDefinition::k_dmxPriorityPropertyId)
		|| changeSet.hasPropertyName(DMXObjectSystemDefinition::k_transmitRateHzPropertyId))
	{
		if (m_dmxManager)
			m_dmxManager->restart(getDMXManagerConfig());
	}
}

void DMXObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<DMXObjectSystem>();
}

void DMXObjectSystem::registerFunctionDescriptors(MikanFunctionDatabasePtr functionDatabase)
{
	functionDatabase->registerFunctionsForSystem<DMXObjectSystem>();
}

// -- IEntityAccessor ----
rfk::Struct const* DMXObjectSystem::getClientAPIValuesStructType() const
{
	return &MikanDMXObjectSystemValues::staticGetArchetype();
}

// -- IPropertyInterface ----
void DMXObjectSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(DMXObjectSystemDefinition::k_dmxPriorityPropertyId,
																  MikanVariantType::UBYTE));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(DMXObjectSystemDefinition::k_transmitRateHzPropertyId,
																  MikanVariantType::FLOAT));
}

bool DMXObjectSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	DMXObjectSystemDefinitionConstPtr def= getDMXObjectSystemConfigConst();

	if (propertyName == DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId)
	{
		outValue= def->getNetworkInterfaceIP();
		return true;
	}
	else if (propertyName == DMXObjectSystemDefinition::k_dmxPriorityPropertyId)
	{
		outValue= def->getDMXPriority();
		return true;
	}
	else if (propertyName == DMXObjectSystemDefinition::k_transmitRateHzPropertyId)
	{
		outValue= def->getTransmitRateHz();
		return true;
	}

	return MikanObjectSystem::getPropertyValue(propertyName, outValue);
}

bool DMXObjectSystem::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	DMXObjectSystemDefinitionPtr def= getDMXObjectSystemConfig();

	if (propertyName == DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId)
	{
		const std::string ip= inValue.getUtf8Value();
		def->setNetworkInterfaceIP(ip);
		return true;
	}
	else if (propertyName == DMXObjectSystemDefinition::k_dmxPriorityPropertyId)
	{
		def->setDMXPriority(inValue.getUByteValue());
		return true;
	}
	else if (propertyName == DMXObjectSystemDefinition::k_transmitRateHzPropertyId)
	{
		def->setTransmitRateHz(inValue.getFloatValue());
		return true;
	}

	return MikanObjectSystem::setPropertyValue(propertyName, inValue);
}

// -- Lua Binding ----
void DMXObjectSystem::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<DMXObjectSystem>("DMXObjectSystem")
		.addFunction("getSpotLightCount",
					 [](DMXObjectSystem* s) -> int
					 {
						 std::vector<MikanComponentPtr> v;
						 s->getComponentList(RGBSpotLightComponent::k_componentClassName, v);
						 return static_cast<int>(v.size());
					 })
		.addFunction("getSpotLightAtIndex",
					 [](DMXObjectSystem* s, int i) -> RGBSpotLightComponent*
					 {
						 std::vector<MikanComponentPtr> v;
						 s->getComponentList(RGBSpotLightComponent::k_componentClassName, v);
						 if (i >= 0 && i < static_cast<int>(v.size()))
							 return std::dynamic_pointer_cast<RGBSpotLightComponent>(v[i]).get();
						 return nullptr;
					 })
		.addFunction("getPixelGridCount",
					 [](DMXObjectSystem* s) -> int
					 {
						 std::vector<MikanComponentPtr> v;
						 s->getComponentList(RGBPixelGridComponent::k_componentClassName, v);
						 return static_cast<int>(v.size());
					 })
		.addFunction("getPixelGridAtIndex",
					 [](DMXObjectSystem* s, int i) -> RGBPixelGridComponent*
					 {
						 std::vector<MikanComponentPtr> v;
						 s->getComponentList(RGBPixelGridComponent::k_componentClassName, v);
						 if (i >= 0 && i < static_cast<int>(v.size()))
							 return std::dynamic_pointer_cast<RGBPixelGridComponent>(v[i]).get();
						 return nullptr;
					 })
		.endClass();
}
