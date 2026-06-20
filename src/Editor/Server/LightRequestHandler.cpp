#include "DMXFixtureComponent.h"
#include "DMXObjectSystem.h"
#include "LightRequestHandler.h"
#include "MikanClientConnectionState.h"
#include "MikanLightEvents.h"
#include "MikanLightRequests.h"
#include "MikanServer.h"
#include "RGBSpotLightSystem.h"
#include "RGBPixelGridSystem.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

bool LightRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Register request handlers
	messageServer->setRequestHandler(
		SetLightDMXDataSubcription::staticGetArchetype().getName(),
		std::bind(&LightRequestHandler::setLightDMXDataSubscriptionHandler, this, _1, _2));
	messageServer->setRequestHandler(
		SetLightDMXData::staticGetArchetype().getName(),
		std::bind(&LightRequestHandler::setLightDMXDataHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetLightDMXData::staticGetArchetype().getName(),
		std::bind(&LightRequestHandler::getLightDMXDataHandler, this, _1, _2));

	// Subscribe to the single DMXObjectSystem delegate for all fixture DMX data changes
	if (auto dmxSystem= getObjectSystemOfType<DMXObjectSystem>())
	{
		dmxSystem->OnDMXDataChanged+= MakeDelegate(this, &LightRequestHandler::onLightDMXDataChanged);
	}

	return true;
}

void LightRequestHandler::shutdown()
{
	if (auto dmxSystem= getObjectSystemOfType<DMXObjectSystem>())
	{
		dmxSystem->OnDMXDataChanged-= MakeDelegate(this, &LightRequestHandler::onLightDMXDataChanged);
	}

	m_lightSubscriptions.clear();
}

void LightRequestHandler::onLightDMXDataChanged(MikanLightID lightId)
{
	auto it= m_lightSubscriptions.find(lightId);
	if (it == m_lightSubscriptions.end() || it->second.empty())
		return;

	auto comp= findLightById(lightId);
	if (!comp)
		return;

	MikanLightDMXDataChangedEvent dmxChangedEvent;
	dmxChangedEvent.light_id= lightId;
	comp->getDMXData(dmxChangedEvent.dmx_data);

	const std::string eventJson= mikanTypeToJsonString(dmxChangedEvent);
	for (const std::string& connectionId : it->second)
	{
		MikanClientConnectionStatePtr clientState= m_owner->getConnectedClientState(connectionId);
		if (clientState)
		{
			clientState->publishMikanJsonEvent(eventJson);
		}
	}
}

void LightRequestHandler::setLightDMXDataSubscriptionHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SetLightDMXDataSubcription subscriptionRequest;
	if (!readTypedRequest(request.utf8RequestString, subscriptionRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const MikanLightID lightId= subscriptionRequest.light_id;
	if (subscriptionRequest.subscribe)
	{
		m_lightSubscriptions[lightId].insert(request.connectionId);
	}
	else
	{
		auto it= m_lightSubscriptions.find(lightId);
		if (it != m_lightSubscriptions.end())
		{
			it->second.erase(request.connectionId);
		}
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void LightRequestHandler::setLightDMXDataHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SetLightDMXData setDataRequest;
	if (!readTypedRequest(request.utf8RequestString, setDataRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto comp= findLightById(setDataRequest.light_id);
	if (!comp)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// setDMXData fires OnDMXDataChanged, which broadcasts to subscribed clients
	comp->setDMXData(setDataRequest.dmx_data);

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void LightRequestHandler::getLightDMXDataHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetLightDMXData getDataRequest;
	if (!readTypedRequest(request.utf8RequestString, getDataRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto comp= findLightById(getDataRequest.light_id);
	if (!comp)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanLightDMXDataResponse dmxResponse;
	dmxResponse.light_id= getDataRequest.light_id;
	comp->getDMXData(dmxResponse.dmx_data);

	writeTypedJsonResponse(request.requestId, dmxResponse, response);
}

std::shared_ptr<DMXFixtureComponent> LightRequestHandler::findLightById(MikanLightID lightId) const
{
	if (auto spotSystem= getObjectSystemOfType<RGBSpotLightSystem>())
	{
		if (auto light= spotSystem->getLightById(lightId))
			return light;
	}

	if (auto gridSystem= getObjectSystemOfType<RGBPixelGridSystem>())
	{
		if (auto grid= gridSystem->getGridById(lightId))
			return grid;
	}

	return nullptr;
}
