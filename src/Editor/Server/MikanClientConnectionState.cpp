#include "InterprocessMessageServerInterface.h"
#include "MikanClientConnectionState.h"
#include "MikanServer.h"
#include "RenderTargetRequestHandler.h"
#include "VRDeviceRequestHandler.h"

MikanClientConnectionState::MikanClientConnectionState(
	MikanServer* ownerServer,
	const std::string& connectionId)
	: m_ownerServer(ownerServer)
	, m_connectionId(connectionId)
	, m_renderTargetClientState(new RenderTargetClientState(this))
	, m_vrDeviceClientState(new VRDeviceClientState(this))
{}

MikanClientConnectionState::~MikanClientConnectionState()
{
	delete m_renderTargetClientState;
	delete m_vrDeviceClientState;
}

const std::string& MikanClientConnectionState::getClientId() const
{
	return m_clientInfo.clientId.getValue();
}

void MikanClientConnectionState::setMikanClientInfo(const MikanClientInfo& clientInfo)
{
	// Free any existing render target
	m_renderTargetClientState->disposeRenderTargetAccessor();

	// Set the new client info describing the client render capabilities
	m_clientInfo = clientInfo;

	// Allocate a new render target accessor
	m_renderTargetClientState->allocateRenderTargetAccessor();
}

void MikanClientConnectionState::clearMikanClientInfo()
{
	// Free any existing render target
	m_renderTargetClientState->disposeRenderTargetAccessor();

	// Reset the client info with defaults
	m_clientInfo = MikanClientInfo();
}

void MikanClientConnectionState::publishMikanJsonEvent(const std::string& mikanJsonEvent)
{
	m_ownerServer->getMessageServer()->sendMessageToClient(m_connectionId, mikanJsonEvent);
}