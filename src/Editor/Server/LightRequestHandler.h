#pragma once

#include "IServerRequestHandler.h"
#include "MikanTypeFwd.h"

#include <map>
#include <set>
#include <string>

class LightRequestHandler : public IServerRequestHandler
{
public:
	LightRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// DMX data change listener — subscribed to DMXFixtureComponent::OnDMXDataChanged
	void onLightDMXDataChanged(MikanLightID lightId);

	// Request handlers
	void setLightDMXDataSubscriptionHandler(const ClientRequest& request, ClientResponse& response);
	void setLightDMXDataHandler(const ClientRequest& request, ClientResponse& response);
	void getLightDMXDataHandler(const ClientRequest& request, ClientResponse& response);

private:
	// Helper: look up a DMXFixtureComponent from either the spot light or pixel grid system
	std::shared_ptr<class DMXFixtureComponent> findLightById(MikanLightID lightId) const;

	// light_id -> set of subscribed client connection IDs
	std::map<MikanLightID, std::set<std::string>> m_lightSubscriptions;
};
