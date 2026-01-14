#pragma once

#include "ComponentFwd.h"
#include "INetworkVideoDeviceManager.h"
#include "MikanTypedObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "NetworkVideoSourceComponent.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceQueries.h"

#include <string>

class NetworkVideoSourceSystemDefinition :
	public MikanTypedObjectSystemDefinition<NetworkVideoSourceComponent, NetworkVideoSourceDefinition, MikanVideoSourceID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<NetworkVideoSourceComponent, NetworkVideoSourceDefinition, MikanVideoSourceID>;

	NetworkVideoSourceSystemDefinition(const std::string& configName = "NetworkVideoSourceSystemDefinition");

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class NetworkVideoSourceSystem :
	public MikanTypedObjectSystem<
		NetworkVideoSourceComponent, NetworkVideoSourceDefinition,
		MikanVideoSourceID,
		NetworkVideoSourceSystem, NetworkVideoSourceSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		NetworkVideoSourceComponent, NetworkVideoSourceDefinition,
		MikanVideoSourceID,
		NetworkVideoSourceSystem, NetworkVideoSourceSystemDefinition>;

    NetworkVideoSourceSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "NetworkVideoObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual void update(float deltaTime) override;
    virtual void dispose() override;

    INetworkVideoDeviceManagerPtr getNetworkVideoDeviceManager() const { return m_networkVideoDeviceManager; }

    VideoSourceIdList getVideoSourceIdList() const;

protected:
	bool ensureNetworkDeviceManager();
	bool createNetworkVideoDeviceManager(const std::string& moduleName);
	void disposeNetworkVideoDeviceManager();

	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		NetworkVideoSourceDefinitionPtr componentDefinition) override;

private:
	class INetworkVideoDeviceModule* m_networkVideoDeviceModule = nullptr;
	INetworkVideoDeviceManagerPtr m_networkVideoDeviceManager = nullptr;
};

using NetworkVideoSourceSystemPtr = std::shared_ptr<NetworkVideoSourceSystem>;
