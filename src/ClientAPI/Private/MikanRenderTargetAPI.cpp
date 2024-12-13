
#include "MikanRenderTargetAPI.h"
#include "MikanRequestManager.h"
#include "MikanRenderTargetRequests.h"
#include "MikanCoreCAPI.h"

MikanRenderTargetAPI::MikanRenderTargetAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{}

MikanAPIResult MikanRenderTargetAPI::setGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void* graphicsDeviceInterface)
{
	MikanContext context = m_requestManager->getContext();

	return (MikanAPIResult)Mikan_SetGraphicsDeviceInterface(context, api, graphicsDeviceInterface);
}

MikanAPIResult MikanRenderTargetAPI::getGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void** outGraphicsDeviceInterface)
{
	MikanContext context = m_requestManager->getContext();

	return (MikanAPIResult)Mikan_GetGraphicsDeviceInterface(context, api, outGraphicsDeviceInterface);
}

bool MikanRenderTargetAPI::tryProcessRequest(MikanRequest& request, MikanResponseFuture& outFuture)
{
	if (typeid(request) == typeid(AllocateRenderTargetTextures))
	{
		outFuture = allocateRenderTargetTextures(request);
		return true;
	}
	else if (typeid(request) == typeid(WriteColorRenderTargetTexture))
	{
		outFuture = writeColorRenderTargetTexture(request);
		return true;
	}
	else if (typeid(request) == typeid(WriteDepthRenderTargetTexture))
	{
		outFuture = writeDepthRenderTargetTexture(request);
		return true;
	}
	else if (typeid(request) == typeid(PublishRenderTargetTextures))
	{
		outFuture = publishRenderTargetTextures(request);
		return true;
	}
	else if (typeid(request) == typeid(FreeRenderTargetTextures))
	{
		outFuture = freeRenderTargetTextures(request);
		return true;
	}

	return false;
}

MikanResponseFuture MikanRenderTargetAPI::allocateRenderTargetTextures(
	MikanRequest& request)
{
	auto& allocateRequest = static_cast<AllocateRenderTargetTextures&>(request);
	const MikanRenderTargetDescriptor& descriptor= allocateRequest.descriptor;

	MikanContext context = m_requestManager->getContext();

	// Create the shared texture
	MikanAPIResult result = (MikanAPIResult)Mikan_AllocateRenderTargetTextures(context, &descriptor);
	if (result == MikanAPIResult::Success)
	{
		// Actual descriptor might differ from desired descriptor based on render target writer's capabilities
		MikanRenderTargetDescriptor actualDescriptor;
		result = (MikanAPIResult)Mikan_GetRenderTargetDescriptor(context, &actualDescriptor);
		if (result == MikanAPIResult::Success)
		{
			return m_requestManager->sendRequest(allocateRequest);
		}
	}

	return m_requestManager->addResponseHandler(INVALID_MIKAN_ID, MikanAPIResult::RequestFailed);
}

MikanResponseFuture MikanRenderTargetAPI::writeColorRenderTargetTexture(
	MikanRequest& request)
{
	auto& writeRequest = static_cast<WriteColorRenderTargetTexture&>(request);
	void* apiColorTexturePtr= writeRequest.apiColorTexturePtr;

	MikanContext context = m_requestManager->getContext();
	MikanAPIResult result= (MikanAPIResult)Mikan_WriteColorRenderTargetTexture(context, apiColorTexturePtr);

	return m_requestManager->makeImmediateResponse(result);
}

MikanResponseFuture MikanRenderTargetAPI::writeDepthRenderTargetTexture(
	MikanRequest& request)
{
	auto& writeRequest = static_cast<WriteDepthRenderTargetTexture&>(request);
	void* apiDepthTexturePtr = writeRequest.apiDepthTexturePtr;
	float zNear= writeRequest.zNear;
	float zFar= writeRequest.zFar;

	MikanContext context = m_requestManager->getContext();
	MikanAPIResult result = 
		(MikanAPIResult)Mikan_WriteDepthRenderTargetTexture(
			context, apiDepthTexturePtr, zNear, zFar);

	return m_requestManager->makeImmediateResponse(result);
}

MikanResponseFuture MikanRenderTargetAPI::publishRenderTargetTextures(
	MikanRequest& request)
{
	auto& publishRequest = static_cast<PublishRenderTargetTextures&>(request);

	return m_requestManager->sendRequest(publishRequest);
}

MikanResponseFuture MikanRenderTargetAPI::freeRenderTargetTextures(
	MikanRequest& request)
{
	auto& freeRequest = static_cast<FreeRenderTargetTextures&>(request);

	// Free any locally allocated resources
	MikanContext context = m_requestManager->getContext();
	Mikan_FreeRenderTargetTextures(context);

	// Tell the server to free the render target resources too
	return m_requestManager->sendRequest(freeRequest);
}