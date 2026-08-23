#pragma once

#include "IServerRequestHandler.h"
#include "MikanTypeFwd.h"

#include <map>
#include <memory>
#include <set>
#include <string>

class LightRequestHandler : public IServerRequestHandler
{
public:
	LightRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// DMX data change listener — subscribed to DMXObjectSystem::OnDMXDataChanged
	void onDMXDataChanged();

	// Request handlers
	void setLightDMXDataSubscriptionHandler(const ClientRequest& request, ClientResponse& response);
	void getDMXDataHandler(const ClientRequest& request, ClientResponse& response);

private:
	static constexpr MikanLightID k_AllLights= -1;

	struct ClientLightSubscriptionInfo
	{
		std::string clientId;
		std::set<MikanLightID> subscribedLights;
	};
	using ClientLightSubscriptionInfoPtr= std::shared_ptr<ClientLightSubscriptionInfo>;

	// Helper: look up which universes contains a set of lights
	void computeDMXUniverseIdsForLights(ClientLightSubscriptionInfoPtr subscriptionInfo,
										std::set<uint16_t>& outUniverseIds) const;

	// Helper: look up a DMXFixtureComponent from either the spot light or pixel grid system
	std::shared_ptr<class DMXFixtureComponent> findLightById(MikanLightID lightId) const;

	// client connection ID -> set of subscribed lights
	std::map<std::string, ClientLightSubscriptionInfoPtr> m_lightSubscriptions;
};
