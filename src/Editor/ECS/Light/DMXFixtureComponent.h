#pragma once

#include "CommonConfig.h"
#include "MikanLightTypes.h"
#include "TransformComponent.h"
#include "LightSystemFwd.h"

#include <cstdint>
#include <string>

class IDMXManager;

// -- DMXFixtureComponentDefinition -----
class DMXFixtureComponentDefinition : public TransformComponentDefinition
{
public:
	DMXFixtureComponentDefinition();
	DMXFixtureComponentDefinition(MikanLightID lightId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;
	virtual bool readFromInitParams(
		MikanObjectSystem* ownerObjectSystem,
		const Serialization::PolymorphicObjectPtr& initParams) override;

	// -- DMX universe / channel address --

	static const std::string k_dmxUniversePropertyId;
	uint16_t getDMXUniverse() const { return m_dmxUniverse; }
	void setDMXUniverse(uint16_t universe);

	static const std::string k_dmxStartChannelPropertyId;
	uint16_t getDMXStartChannel() const { return m_dmxStartChannel; }
	void setDMXStartChannel(uint16_t startChannel);

	static const std::string k_dmxChannelCountPropertyId;
	uint16_t getDMXChannelCount() const { return m_dmxChannelCount; }

	static const std::string k_isDisabledPropertyId;
	bool getIsDisabled() const { return m_bIsDisabled; }
	void setIsDisabled(bool flag);

protected:
	void setDMXChannelCount(uint16_t count);

	uint16_t m_dmxUniverse = 1;
	uint16_t m_dmxStartChannel = 1;
	uint16_t m_dmxChannelCount = 3;
	bool m_bIsDisabled = false;
};

// -- DMXFixtureComponent -----
class DMXFixtureComponent : public TransformComponent
{
public:
	DMXFixtureComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "DMXFixtureComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline DMXFixtureComponentDefinitionPtr getDMXFixtureDefinition() const
	{
		return std::static_pointer_cast<DMXFixtureComponentDefinition>(m_definition);
	}

	virtual void init() override;

	/// Write this fixture's current channel values to the DMX manager.
	/// Called each frame by the owning object system.
	virtual void sendDMXData(IDMXManager* manager) const = 0;

	/// Get the current DMX channel data as a flat byte array.
	virtual void getDMXData(MikanDMXData& outData) const = 0;

	/// Apply a flat byte array of DMX channel data to this fixture.
	/// Implementations should notify DMXObjectSystem::OnDMXDataChanged after updating.
	virtual void setDMXData(const MikanDMXData& data) = 0;

protected:
	/// Cached pointer to the DMXObjectSystem for firing OnDMXDataChanged. Set during init().
	DMXObjectSystemWeakPtr m_dmxObjectSystem;

	void notifyDMXDataChanged();

	// -- IEntityAccessor --
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface --
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- Lua Binding --
	static void bindLuaFunctions(struct lua_State* L);
};
