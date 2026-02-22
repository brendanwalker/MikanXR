#include "ClientSourceManager.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MikanClientConnectionState.h"
#include "MikanServer.h"
#include "SharedTextureReader.h"
#include "CameraRequestHandler.h"

bool ClientSourceManager::startup(IMkWindow* ownerWindow)
{
	m_ownerWindow= ownerWindow;

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

		clientSource->colorTexture = nullptr;
		clientSource->depthTexture = nullptr;

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
	eTextureSourceColorType textureSourceColorType) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr)
	{
		switch (textureSourceColorType)
		{
			case eTextureSourceColorType::colorRGB:
				if (clientSource->colorTexture &&
					(clientSource->colorTexture->getTextureFormat() == MK_RGB ||
						clientSource->colorTexture->getTextureFormat() == MK_BGR))
				{
					return clientSource->colorTexture;
				}
				return clientSource->colorTexture;
			case eTextureSourceColorType::colorRGBA:
				if (clientSource->colorTexture &&
					(clientSource->colorTexture->getTextureFormat() == MK_RGBA ||
						clientSource->colorTexture->getTextureFormat() == MK_BGRA))
				{
					return clientSource->colorTexture;
				}
		}
	}

	return IMkTexturePtr();
}

IMkTexturePtr ClientSourceManager::getClientDepthSourceTexture(
	const std::string& clientId,
	MikanCameraID cameraId,
	eTextureSourceDepthType textureSourceColorType) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr)
	{
		switch (textureSourceColorType)
		{
			case eTextureSourceDepthType::depthPackRGBA:
				return clientSource->depthTexture;
		}
	}

	return IMkTexturePtr();
}

bool ClientSourceManager::getIsSourcePendingRender(
	const std::string& clientId,
	MikanCameraID cameraId) const
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr)
	{
		return clientSource->bIsPendingRender;
	}

	return false;
}

bool ClientSourceManager::markSourceAsPendingRender(const std::string& clientId, MikanCameraID cameraId)
{
	ClientSource* clientSource= getClientSource(clientId, cameraId);
	if (clientSource != nullptr)
	{
		clientSource->bIsPendingRender= true;
		return true;
	}

	return false;
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
	ClientSourceManager::ClientSource* clientSource = nullptr;
	const std::string tableKey= makeClientSourceTableKey(clientId, cameraId);
	if (m_clientSources.tryGetValue(tableKey, clientSource))
	{
		return clientSource;
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
	clientSource->frameIndex = 0;

	switch (desc.color_buffer_type)
	{
		case MikanColorBuffer_RGB24:
			clientSource->colorTexture = CreateMkTexture();
			clientSource->colorTexture->setTextureFormat(MK_RGB);
			clientSource->colorTexture->setBufferFormat(MK_RGB);
			break;
		case MikanColorBuffer_RGBA32:
			clientSource->colorTexture = CreateMkTexture();
			clientSource->colorTexture->setTextureFormat(MK_RGBA);
			clientSource->colorTexture->setBufferFormat(MK_RGBA);
			break;
		case MikanColorBuffer_BGRA32:
			clientSource->colorTexture = CreateMkTexture();
			clientSource->colorTexture->setTextureFormat(MK_RGBA);
			clientSource->colorTexture->setBufferFormat(MK_BGRA);
			break;
	}

	if (clientSource->colorTexture != nullptr)
	{
		clientSource->colorTexture->setSize(desc.width, desc.height);
		clientSource->colorTexture->setGenerateMipMap(false);
		clientSource->colorTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsApi_UNKNOWN
			? IMkTexture::PixelBufferObjectMode::DoublePBOWrite
			: IMkTexture::PixelBufferObjectMode::NoPBO);
		bSuccess&= clientSource->colorTexture->createTexture();

		readAccessor->setColorTexture(clientSource->colorTexture);
	}

	switch (desc.depth_buffer_type)
	{
		case MikanDepthBuffer_FLOAT_DEVICE_DEPTH:
		case MikanDepthBuffer_FLOAT_SCENE_DEPTH:
			clientSource->depthTexture = CreateMkTexture();
			clientSource->depthTexture->setTextureFormat(MK_R32F);
			clientSource->depthTexture->setBufferFormat(MK_RED);
			break;
		case MikanDepthBuffer_PACK_DEPTH_RGBA:
			clientSource->depthTexture = CreateMkTexture();
			clientSource->depthTexture->setTextureFormat(MK_RGBA);
			clientSource->depthTexture->setBufferFormat(MK_RGBA);
			break;
	}

	if (clientSource->depthTexture != nullptr)
	{
		clientSource->depthTexture->setSize(desc.width, desc.height);
		clientSource->depthTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsApi_UNKNOWN
			? IMkTexture::PixelBufferObjectMode::DoublePBOWrite
			: IMkTexture::PixelBufferObjectMode::NoPBO);
		bSuccess &= clientSource->depthTexture->createTexture();

		readAccessor->setDepthTexture(clientSource->depthTexture);
	}

	if (bSuccess)
	{
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
		clientSource->colorTexture = nullptr;
		readAccessor->setColorTexture(nullptr);

		clientSource->depthTexture = nullptr;
		readAccessor->setDepthTexture(nullptr);

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

	clientSource->colorTexture = nullptr;
	readAccessor->setColorTexture(nullptr);

	clientSource->depthTexture = nullptr;
	readAccessor->setDepthTexture(nullptr);

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
	if (clientSource != nullptr)
	{
		// Update the frame index
		clientSource->frameIndex = frameIndex;

		// Mark that the client source is no longer pending the render
		//assert(clientSource->bIsPendingRender);
		clientSource->bIsPendingRender = false;
	}
}