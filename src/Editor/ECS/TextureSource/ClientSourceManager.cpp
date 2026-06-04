#include "ClientSourceManager.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MikanClientConnectionState.h"
#include "MikanServer.h"
#include "SharedTextureReader.h"
#include "CameraRequestHandler.h"

ClientSourceManager::ClientSourceManager(int textureQueueSize)
	: m_textureQueueSize(textureQueueSize)
{
}

bool ClientSourceManager::startup()
{
	MikanServer* mikanServer = MikanServer::getInstance();

	// Create layers for all connected clients with allocated render targets
	std::vector<MikanClientConnectionStateConstPtr> clientStateList;
	mikanServer->getConnectedClientStateList(clientStateList);
	for (MikanClientConnectionStateConstPtr clientState : clientStateList)
	{
		RenderTargetClientState* renderTargetClientState = clientState->getRenderTargetClientState();
		const SharedTextureReadAccessorCameraMap& readAccessorMap= renderTargetClientState->getRenderTargetAccessorMap();

		for (auto it : readAccessorMap)
		{
			SharedTextureReadAccessorPtr readAccessor= it.second;

			onClientRenderTargetAllocated(
				clientState->getClientId(),
				clientState->getMikanClientInfo(),
				readAccessor.get());
		}
	}

	// Listen for new render target events
	auto* cameraRequestHandler = mikanServer->getCameraRequestHandler();
	cameraRequestHandler->OnClientRenderTargetAllocated += MakeDelegate(this, &ClientSourceManager::onClientRenderTargetAllocated);
	cameraRequestHandler->OnClientRenderTargetReleased += MakeDelegate(this, &ClientSourceManager::onClientRenderTargetReleased);
	cameraRequestHandler->OnClientRenderTargetUpdated += MakeDelegate(this, &ClientSourceManager::onClientRenderTargetUpdated);

	return true;
}

void ClientSourceManager::shutdown()
{
	// Stop listening to render target events
	MikanServer* mikanServer = MikanServer::getInstance();
	auto* cameraRequestHandler = mikanServer->getCameraRequestHandler();

	cameraRequestHandler->OnClientRenderTargetAllocated -= MakeDelegate(this, &ClientSourceManager::onClientRenderTargetAllocated);
	cameraRequestHandler->OnClientRenderTargetReleased -= MakeDelegate(this, &ClientSourceManager::onClientRenderTargetReleased);
	cameraRequestHandler->OnClientRenderTargetUpdated -= MakeDelegate(this, &ClientSourceManager::onClientRenderTargetUpdated);

	// Clean up any allocated clientSources
	for (auto iter = m_clientSources.getMap().begin(); iter != m_clientSources.getMap().end(); iter++)
	{
		ClientSource* clientSource = iter->second;

		if (clientSource->textureQueue != nullptr)
		{
			clientSource->textureQueue->dispose();
			delete clientSource->textureQueue;
			clientSource->textureQueue = nullptr;
		}
		clientSource->readAccessor = nullptr;

		delete clientSource;
	}
	m_clientSources.clear();
}

bool ClientSourceManager::hasClientSource(
	const std::string& clientId, 
	MikanCameraID cameraId) const
{
	return getClientSource(clientId, cameraId) != nullptr;
}

bool ClientSourceManager::getClientSourceDimensions(
	const std::string& clientId, 
	MikanCameraID cameraId,
	int& outWidth, int& outHeight) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr)
	{
		outWidth = clientSource->desc.width;
		outHeight = clientSource->desc.height;
		return true;
	}

	return false;
}

IMkTexturePtr ClientSourceManager::getClientColorSourceTexture(
	const std::string& clientId,
	MikanCameraID cameraId,
	eTextureSourceColorType textureSourceColorType,
	int64_t frameIndex) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr && clientSource->textureQueue != nullptr)
	{
		switch (textureSourceColorType)
		{
			case eTextureSourceColorType::colorRGB:
			case eTextureSourceColorType::colorRGBA:
				return clientSource->textureQueue->getColorTexture(frameIndex);
		}
	}

	return IMkTexturePtr();
}

IMkTexturePtr ClientSourceManager::getClientDepthSourceTexture(
	const std::string& clientId,
	MikanCameraID cameraId,
	eTextureSourceDepthType textureSourceDepthType,
	int64_t frameIndex) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr && clientSource->textureQueue != nullptr)
	{
		switch (textureSourceDepthType)
		{
			case eTextureSourceDepthType::depthPackRGBA:
				return clientSource->textureQueue->getDepthTexture(frameIndex);
		}
	}

	return IMkTexturePtr();
}

std::string ClientSourceManager::makeClientSourceTableKey(
	const std::string& clientId,
	MikanCameraID cameraId)
{
	return clientId + "_camera" + std::to_string(cameraId);
}

ClientSourceManager::ClientSource* ClientSourceManager::getClientSource(
	const std::string& clientId,
	MikanCameraID cameraId) const
{
	if (cameraId != INVALID_MIKAN_ID)
	{
		ClientSourceManager::ClientSource* clientSource = nullptr;
		const std::string tableKey = makeClientSourceTableKey(clientId, cameraId);
		if (m_clientSources.tryGetValue(tableKey, clientSource))
		{
			return clientSource;
		}
	}
	else
	{
		// If an invalid cameraId was provided, find the newest client source for the clientId regardless of cameraId. 
		int64_t newestFrameIndex = -1;
		ClientSourceManager::ClientSource* newestClientSource = nullptr;
		for (auto iter = m_clientSources.getMap().begin(); iter != m_clientSources.getMap().end(); iter++)
		{
			ClientSourceManager::ClientSource* clientSource = iter->second;
			if (clientSource->clientId == clientId && clientSource->frameIndex > newestFrameIndex)
			{
				newestFrameIndex = clientSource->frameIndex;
				newestClientSource = clientSource;
			}
		}

		return newestClientSource;
	}

	return nullptr;
}

bool ClientSourceManager::addClientSource(
	const std::string& clientId,
	const MikanClientInfo& clientInfo,
	SharedTextureReadAccessor* readAccessor)
{
	MikanCameraID cameraId = readAccessor->getCameraId();
	if (getClientSource(clientId, cameraId) != nullptr)
		return false;

	ClientSource* clientSource = new ClientSource();
	bool bSuccess = true;

	const MikanRenderTargetDescriptor& desc = readAccessor->getRenderTargetDescriptor();
	clientSource->clientId = clientId;
	clientSource->clientInfo = clientInfo;
	clientSource->desc = desc;
	clientSource->readAccessor = readAccessor;
	clientSource->frameIndex = 0;

	// Create the circular texture frame queue
	clientSource->textureQueue = new ClientTextureFrameQueue(m_textureQueueSize);
	bSuccess = clientSource->textureQueue->initialize(desc);

	if (bSuccess)
	{
		// Point the read accessor at the first pending write slot
		readAccessor->setColorTexture(clientSource->textureQueue->getPendingWriteColorTexture());
		readAccessor->setDepthTexture(clientSource->textureQueue->getPendingWriteDepthTexture());

		// Add the client source to the data source table
		const std::string tableKey = makeClientSourceTableKey(clientId, cameraId);
		m_clientSources.setValue(tableKey, clientSource);

		// Notify listeners that a new client source has connected
		if (OnClientSourceConnected)
		{
			OnClientSourceConnected(clientId, cameraId);
		}
	}
	else
	{
		// Clean up on failure
		readAccessor->setColorTexture(nullptr);
		readAccessor->setDepthTexture(nullptr);

		clientSource->textureQueue->dispose();
		delete clientSource->textureQueue;
		clientSource->readAccessor = nullptr;

		delete clientSource;
	}

	return bSuccess;
}

bool ClientSourceManager::removeClientSource(
	const std::string& clientId,
	SharedTextureReadAccessor* readAccessor)
{
	MikanCameraID cameraId = readAccessor->getCameraId();
	ClientSource* clientSource = getClientSource(clientId, cameraId);
	if (clientSource == nullptr)
		return false;

	readAccessor->setColorTexture(nullptr);
	readAccessor->setDepthTexture(nullptr);

	clientSource->textureQueue->dispose();
	delete clientSource->textureQueue;
	clientSource->textureQueue = nullptr;
	clientSource->readAccessor = nullptr;

	// Remove the client source entries from the data source tables
	const std::string tableKey = makeClientSourceTableKey(clientId, cameraId);
	m_clientSources.removeValue(tableKey);

	delete clientSource;

	// Notify listeners that a new client source has disconnected
	if (OnClientSourceDisconnected)
	{
		OnClientSourceDisconnected(clientId, cameraId);
	}

	return true;
}

// MikanServer Events
void ClientSourceManager::onClientRenderTargetAllocated(
	const std::string& clientId,
	const MikanClientInfo& clientInfo,
	SharedTextureReadAccessor* readAccessor)
{
	MIKAN_LOG_TRACE("ClientSourceManager::onClientRenderTargetAllocated") << "Adding Source " << clientId;

	addClientSource(clientId, clientInfo, readAccessor);
}

void ClientSourceManager::onClientRenderTargetReleased(
	const std::string& clientId,
	SharedTextureReadAccessor* readAccessor)
{
	MIKAN_LOG_TRACE("ClientSourceManager::onClientRenderTargetAllocated") << "Removing Source " << clientId;

	removeClientSource(clientId, readAccessor);
}

void ClientSourceManager::onClientRenderTargetUpdated(
	const std::string& clientId,
	MikanCameraID cameraId,
	int64_t frameIndex)
{
	MIKAN_LOG_TRACE("ClientSourceManager::onClientRenderTargetUpdated") << "Recv frame " << frameIndex;

	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr && clientSource->textureQueue != nullptr)
	{
		// Stamp the frame index on the current slot and advance to the next
		clientSource->textureQueue->advanceWriteIndex(frameIndex);
		clientSource->frameIndex = frameIndex;

		// Re-point the accessor at the new pending write slot
		if (clientSource->readAccessor != nullptr)
		{
			clientSource->readAccessor->setColorTexture(clientSource->textureQueue->getPendingWriteColorTexture());
			clientSource->readAccessor->setDepthTexture(clientSource->textureQueue->getPendingWriteDepthTexture());
		}
	}
}