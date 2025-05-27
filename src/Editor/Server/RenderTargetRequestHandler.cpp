#include "App.h"
#include "AppStage.h"
#include "RenderTargetRequestHandler.h"
#include "MainWindow.h"
#include "MikanClientConnectionState.h"
#include "MikanServer.h"
#include "MikanRenderTargetRequests.h"
#include "ServerResponseHelpers.h"
#include "SharedTextureReader.h"

#include <functional>

using namespace std::placeholders;

// -- RenderTargetClientState ----- 
RenderTargetClientState::RenderTargetClientState(class MikanClientConnectionState* owner)
	: m_owner(owner)
	, m_renderTargetReadAccessor(nullptr)
{}

RenderTargetClientState::~RenderTargetClientState()
{
	disposeRenderTargetAccessor();
}

MikanClientGraphicsApi RenderTargetClientState::getClientGraphicsAPI() const
{
	return m_renderTargetReadAccessor->getClientGraphicsAPI();
}

void RenderTargetClientState::allocateRenderTargetAccessor()
{
	assert(m_renderTargetReadAccessor == nullptr);
	m_renderTargetReadAccessor = new SharedTextureReadAccessor(m_owner->getClientId());
}

void RenderTargetClientState::disposeRenderTargetAccessor()
{
	if (m_renderTargetReadAccessor != nullptr)
	{
		delete m_renderTargetReadAccessor;
		m_renderTargetReadAccessor = nullptr;
	}
}

bool RenderTargetClientState::hasAllocatedRenderTarget() const
{
	if (m_renderTargetReadAccessor != nullptr)
	{
		const MikanRenderTargetDescriptor& desc = m_renderTargetReadAccessor->getRenderTargetDescriptor();

		return
			desc.color_buffer_type != MikanColorBuffer_NOCOLOR ||
			desc.depth_buffer_type != MikanDepthBuffer_NODEPTH;
	}

	return false;
}

bool RenderTargetClientState::allocateRenderTargetTextures(const MikanRenderTargetDescriptor& desc)
{
	// This will free any existing render target
	if (m_renderTargetReadAccessor != nullptr &&
		m_renderTargetReadAccessor->initialize(&desc))
	{
		auto* renderTargetHandler = MikanServer::getInstance()->getRenderTargetRequestHandler();

		if (renderTargetHandler->OnClientRenderTargetAllocated)
		{
			renderTargetHandler->OnClientRenderTargetAllocated(
				m_owner->getClientId(),
				m_owner->getMikanClientInfo(),
				m_renderTargetReadAccessor);
		}
	}

	return false;
}

void RenderTargetClientState::freeRenderTargetTexturesHandler()
{
	if (hasAllocatedRenderTarget())
	{
		auto* renderTargetHandler = MikanServer::getInstance()->getRenderTargetRequestHandler();

		if (renderTargetHandler->OnClientRenderTargetReleased)
		{
			renderTargetHandler->OnClientRenderTargetReleased(
				m_owner->getClientId(),
				m_renderTargetReadAccessor);
		}

		m_renderTargetReadAccessor->dispose();
	}
}

bool RenderTargetClientState::readRenderTargetTextures(const int64_t newFrameIndex)
{
	SharedTextureReadAccessor* readAccessor = getRenderTargetReadAccessor();
	if (readAccessor)
	{
		return readAccessor->readRenderTargetTextures(newFrameIndex);
	}

	return false;
}

// -- RenderTargetRequestHandler -----
bool RenderTargetRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Render Target Requests
	messageServer->setRequestHandler(
		AllocateRenderTargetTextures::staticGetArchetype().getId(),
		std::bind(&RenderTargetRequestHandler::allocateRenderTargetTexturesHandler, this, _1, _2));
	messageServer->setRequestHandler(
		FreeRenderTargetTextures::staticGetArchetype().getId(),
		std::bind(&RenderTargetRequestHandler::freeRenderTargetTexturesHandler, this, _1, _2));
	messageServer->setRequestHandler(
		PublishRenderTargetTextures::staticGetArchetype().getId(),
		std::bind(&RenderTargetRequestHandler::frameRenderedHandler, this, _1, _2));


	return true;
}

void RenderTargetRequestHandler::allocateRenderTargetTexturesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	AllocateRenderTargetTextures allocateRequest;
	if (!readTypedRequest(request.utf8RequestString, allocateRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = m_owner->getConnectedClientState(request.connectionId);
	if (!clientState)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();
	if (renderTargetState->allocateRenderTargetTextures(allocateRequest.descriptor))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
	}
}

void RenderTargetRequestHandler::freeRenderTargetTexturesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanClientConnectionStatePtr clientState = m_owner->getConnectedClientState(request.connectionId);
	if (clientState)
	{
		RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();

		// Broadcast that the client render target was disposed
		if (OnClientRenderTargetReleased)
		{
			OnClientRenderTargetReleased(
				clientState->getClientId(),
				renderTargetState->getRenderTargetReadAccessor());
		}

		// Free the render target texture
		renderTargetState->freeRenderTargetTexturesHandler();

		// Send response back to the client
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

void RenderTargetRequestHandler::frameRenderedHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	PublishRenderTargetTextures frameRenderedRequest = {};
	if (!readTypedRequest(request.utf8RequestString, frameRenderedRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = m_owner->getConnectedClientState(request.connectionId);
	if (clientState)
	{
		RenderTargetClientState* renderTargetState= clientState->getRenderTargetClientState();

		// Process incoming video frames, if we have a compositor active
		if (OnClientRenderTargetUpdated)
		{
			if (renderTargetState->readRenderTargetTextures(frameRenderedRequest.frameIndex))
			{
				OnClientRenderTargetUpdated(
					clientState->getClientId(), 
					frameRenderedRequest.frameIndex);
			}
		}

		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}