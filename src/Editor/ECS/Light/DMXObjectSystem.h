#pragma once

#include "MikanObjectSystem.h"
#include "DMXManagerConfig.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IDMXManager.h"
#include "LightSystemFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

class DMXObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	DMXObjectSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator)
		: MikanObjectSystemDefinition(configName, idAllocator)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	// -- DMX sender config --
	static const std::string k_networkInterfaceIPPropertyId;
	const std::string& getNetworkInterfaceIP() const { return m_dmxConfig.networkInterfaceIP; }
	void setNetworkInterfaceIP(const std::string& ip);

	static const std::string k_dmxPriorityPropertyId;
	uint8_t getDMXPriority() const { return m_dmxConfig.priority; }
	void setDMXPriority(uint8_t priority);

	static const std::string k_transmitRateHzPropertyId;
	float getTransmitRateHz() const { return m_dmxConfig.transmitRateHz; }
	void setTransmitRateHz(float hz);

	const DMXManagerConfig& getDMXManagerConfig() const { return m_dmxConfig; }

private:
	DMXManagerConfig m_dmxConfig;
};

class DMXObjectSystem : public MikanObjectSystem
{
public:
	DMXObjectSystem(ProjectManagerPtr ownerObjectSystem)
		: MikanObjectSystem(ownerObjectSystem)
	{
	}

	inline static const std::string k_objectSystemClassName= "DMXObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;

	DMXObjectSystemDefinitionConstPtr getDMXObjectSystemConfigConst() const;
	DMXObjectSystemDefinitionPtr getDMXObjectSystemConfig();
	const DMXManagerConfig& getDMXManagerConfig() const
	{
		return getDMXObjectSystemConfigConst()->getDMXManagerConfig();
	}

	virtual MikanComponentPtr getComponentById(int componentId) const override;
	virtual bool getComponentList(const std::string& componentClassName,
								  std::vector<MikanComponentPtr>& outComponentList) const override;
	virtual bool getComponentIdList(const std::string& componentClassName,
									std::vector<int>& outComponentIdList) const override;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;
	virtual void registerFunctionDescriptors(MikanFunctionDatabasePtr functionDatabase) override;

	// DMX Universe Data helpers
	void writeUniverseData(uint16_t universeId, uint16_t startChannel, const uint8_t* values, uint16_t count);
	std::set<uint16_t> getActiveDMXUniverseIdSet() const;
	bool extractUniverseData(uint16_t universeId, struct MikanUniverseDMXData& outUniverseData);

	/// Fired when DMX Universe data changed.
	MulticastDelegate<void()> OnDMXDataChanged;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static const std::string k_selectedLanguagePropertyId;
	static const std::string k_availableLanguageListPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

private:
	static constexpr size_t kDMXUniverseChannelCount= 512;
	struct UniverseData
	{
		uint16_t universeId;
		uint8_t channelData[kDMXUniverseChannelCount]= {}; // DMX channel data (slots 1-512)
		bool dirty= false;
	};
	using UniverseDataPtr= std::shared_ptr<UniverseData>;
	std::map<uint16_t, UniverseDataPtr> m_universeBuffers;

	IDMXManagerUniquePtr m_dmxManager;

protected:
	UniverseDataPtr getOrAddUniverseData(uint16_t universeId);
	void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
};