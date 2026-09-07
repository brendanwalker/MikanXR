#include "App.h"
#include "AppStage.h"
#include "CameraRequestHandler.h"
#include "MainWindow.h"
#include "MikanClientConnectionState.h"
#include "MikanServer.h"
#include "MikanCameraRequests.h"
#include "ServerResponseHelpers.h"
#include "SharedTextureReader.h"

#include <functional>

using namespace std::placeholders;

// -- RenderTargetClientState -----
RenderTargetClientState::RenderTargetClientState(class MikanClientConnectionState* owner)
	: m_owner(owner)
{
}

RenderTargetClientState::~RenderTargetClientState() { disposeAllRenderTargetAccessors(); }

MikanClientGraphicsApi RenderTargetClientState::getClientGraphicsAPI(MikanCameraID cameraId) const
{
	SharedTextureReadAccessor* readAccessor= getRenderTargetReadAccessor(cameraId);

	return readAccessor ? readAccessor->getClientGraphicsAPI() : MikanClientGraphicsApi_UNKNOWN;
}

SharedTextureReadAccessor* RenderTargetClientState::getOrAllocateRenderTargetAccessor(
	MikanCameraID cameraId, const MikanRenderTargetDescriptor& desc)
{
	SharedTextureReadAccessor* readAccessor= getRenderTargetReadAccessor(cameraId);

	if (readAccessor == nullptr)
	{
		SharedTextureReadAccessorPtr newReadAccessorPtr=
			std::make_shared<SharedTextureReadAccessor>(m_owner->getClientId(), cameraId);

		m_renderTargetReadAccessorCameraMap.insert({cameraId, newReadAccessorPtr});
		readAccessor= newReadAccessorPtr.get();
	}

	return readAccessor;
}

// The one place a render target accessor goes away. Every teardown path routes through here
// (an explicit free request, a dropped connection, the connection state's destructor) so that
// listeners always see the release. A listener that misses one keeps a source bound to a dead
// accessor, and the client's next allocation has nowhere to land.
void RenderTargetClientState::disposeRenderTargetAccessor(MikanCameraID cameraId)
{
	auto it= m_renderTargetReadAccessorCameraMap.find(cameraId);
	if (it != m_renderTargetReadAccessorCameraMap.end())
	{
		SharedTextureReadAccessorPtr readAccessor= it->second;

		notifyRenderTargetReleased(readAccessor.get());

		// invokes SharedTextureReadAccessor::dispose upon removal of SharedTextureReadAccessor
		m_renderTargetReadAccessorCameraMap.erase(it);
	}
}

void RenderTargetClientState::disposeAllRenderTargetAccessors()
{
	// Take the first entry each pass rather than iterating: a listener is free to reach back
	// into this state while being notified.
	while (!m_renderTargetReadAccessorCameraMap.empty())
	{
		disposeRenderTargetAccessor(m_renderTargetReadAccessorCameraMap.begin()->first);
	}
}

void RenderTargetClientState::notifyRenderTargetReleased(SharedTextureReadAccessor* readAccessor)
{
	if (readAccessor == nullptr)
		return;

	// The server owns the connection states, so it outlives them on every ordinary path. The
	// destructor can still run during teardown, after the request handler is gone.
	MikanServer* mikanServer= MikanServer::getInstance();
	auto* cameraRequestHandler= mikanServer != nullptr ? mikanServer->getCameraRequestHandler() : nullptr;

	if (cameraRequestHandler != nullptr && cameraRequestHandler->OnClientRenderTargetReleased)
	{
		cameraRequestHandler->OnClientRenderTargetReleased(m_owner->getClientId(), readAccessor);
	}
}

class SharedTextureReadAccessor* RenderTargetClientState::getRenderTargetReadAccessor(MikanCameraID cameraId) const
{
	auto it= m_renderTargetReadAccessorCameraMap.find(cameraId);
	if (it != m_renderTargetReadAccessorCameraMap.end())
	{
		SharedTextureReadAccessorPtr readAccessor= it->second;

		return readAccessor.get();
	}

	return nullptr;
}

bool RenderTargetClientState::hasAllocatedRenderTarget(MikanCameraID cameraId) const
{
	SharedTextureReadAccessor* readAccessor= getRenderTargetReadAccessor(cameraId);

	if (readAccessor != nullptr)
	{
		const MikanRenderTargetDescriptor& desc= readAccessor->getRenderTargetDescriptor();

		return desc.color_buffer_type != MikanColorBuffer_NOCOLOR || desc.depth_buffer_type != MikanDepthBuffer_NODEPTH;
	}

	return false;
}

bool RenderTargetClientState::allocateRenderTargetTextures(MikanCameraID cameraId,
														   const MikanRenderTargetDescriptor& desc)
{
	SharedTextureReadAccessor* readAccessor= getOrAllocateRenderTargetAccessor(cameraId, desc);

	// This will free any existing render target
	if (readAccessor != nullptr && readAccessor->initialize(&desc))
	{
		auto* cameraRequestHandler= MikanServer::getInstance()->getCameraRequestHandler();

		if (cameraRequestHandler->OnClientRenderTargetAllocated)
		{
			cameraRequestHandler->OnClientRenderTargetAllocated(m_owner->getClientId(), m_owner->getMikanClientInfo(),
																readAccessor);
		}

		return true;
	}

	return false;
}

bool RenderTargetClientState::readRenderTargetTextures(MikanCameraID cameraId, const int64_t newFrameIndex)
{
	SharedTextureReadAccessor* readAccessor= getRenderTargetReadAccessor(cameraId);
	if (readAccessor)
	{
		return readAccessor->readRenderTargetTextures(newFrameIndex);
	}

	return false;
}

// --RenderTargetRequestHandler ----
bool CameraRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Render Target Requests
	messageServer->setRequestHandler(
		AllocateCameraRenderTargetTextures::staticGetArchetype().getName(),
		std::bind(&CameraRequestHandler::allocateRenderTargetTexturesHandler, this, _1, _2));
	messageServer->setRequestHandler(FreeCameraRenderTargetTextures::staticGetArchetype().getName(),
									 std::bind(&CameraRequestHandler::freeRenderTargetTexturesHandler, this, _1, _2));
	messageServer->setRequestHandler(PublishCameraRenderTargetTextures::staticGetArchetype().getName(),
									 std::bind(&CameraRequestHandler::frameRenderedHandler, this, _1, _2));

	return true;
}

void CameraRequestHandler::allocateRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response)
{
	AllocateCameraRenderTargetTextures allocateRequest;
	if (!readTypedRequest(request.utf8RequestString, allocateRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState= m_owner->getConnectedClientState(request.connectionId);
	if (!clientState)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();
	if (renderTargetState->allocateRenderTargetTextures(allocateRequest.camera_id, allocateRequest.descriptor))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
	}
}

void CameraRequestHandler::freeRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response)
{
	FreeCameraRenderTargetTextures freeRenderTargetTexturesRequest= {};
	if (!readTypedRequest(request.utf8RequestString, freeRenderTargetTexturesRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState= m_owner->getConnectedClientState(request.connectionId);
	if (clientState)
	{
		RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();

		// Free the render target texture, which broadcasts the release to listeners
		renderTargetState->disposeRenderTargetAccessor(freeRenderTargetTexturesRequest.camera_id);

		// Send response back to the client
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

void CameraRequestHandler::frameRenderedHandler(const ClientRequest& request, ClientResponse& response)
{
	PublishCameraRenderTargetTextures frameRenderedRequest= {};
	if (!readTypedRequest(request.utf8RequestString, frameRenderedRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState= m_owner->getConnectedClientState(request.connectionId);
	if (clientState)
	{
		RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();

		// Process incoming video frames, if we have a compositor active
		if (OnClientRenderTargetUpdated)
		{
			if (renderTargetState->readRenderTargetTextures(frameRenderedRequest.camera_id,
															frameRenderedRequest.frame_index))
			{
				OnClientRenderTargetUpdated(clientState->getClientId(), frameRenderedRequest.camera_id,
											frameRenderedRequest.frame_index);
			}
		}

		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

// -- Camera Events -----
void CameraRequestHandler::publishCameraNewFrameEvent(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(newFrameEvent));
}