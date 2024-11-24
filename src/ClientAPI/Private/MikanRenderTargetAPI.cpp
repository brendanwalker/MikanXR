
#include "MikanRenderTargetAPI.h"
#include "MikanRequestManager.h"
#include "MikanRenderTargetRequests.h"
#include "MikanCoreCAPI.h"

MikanRenderTargetAPI::MikanRenderTargetAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{}

MikanResult MikanRenderTargetAPI::setGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void* graphicsDeviceInterface)
{
	MikanContext context = m_requestManager->getContext();

	return Mikan_SetGraphicsDeviceInterface(context, api, graphicsDeviceInterface);
}

MikanResult MikanRenderTargetAPI::getGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void** outGraphicsDeviceInterface)
{
	MikanContext context = m_requestManager->getContext();

	return Mikan_GetGraphicsDeviceInterface(context, api, outGraphicsDeviceInterface);
}

bool MikanRenderTargetAPI::tryProcessRequest(const MikanRequest& request, MikanResponseFuture& outFuture)
{
	if (request.requestType.getValue() == "allocateRenderTargetTextures")
	{
		outFuture = allocateRenderTargetTextures(request);
		return true;
	}
	else if (request.requestType.getValue() == "writeColorRenderTargetTexture")
	{
		outFuture = writeColorRenderTargetTexture(request);
		return true;
	}
	else if (request.requestType.getValue() == "writeDepthRenderTargetTexture")
	{
		outFuture = writeDepthRenderTargetTexture(request);
		return true;
	}
	else if (request.requestType.getValue() == "publishRenderTargetTextures")
	{
		outFuture = publishRenderTargetTextures(request);
		return true;
	}
	else if (request.requestType.getValue() == "freeRenderTargetTextures")
	{
		outFuture = freeRenderTargetTextures(request);
		return true;
	}

	return false;
}

MikanResponseFuture MikanRenderTargetAPI::allocateRenderTargetTextures(
	const MikanRequest& request)
{
	auto& allocateRequest = static_cast<const AllocateRenderTargetTextures&>(request);
	const MikanRenderTargetDescriptor& descriptor= allocateRequest.descriptor;

	MikanContext context = m_requestManager->getContext();

	// Create the shared texture
	MikanResult result = Mikan_AllocateRenderTargetTextures(context, &descriptor);
	if (result == MikanResult_Success)
	{
		// Actual descriptor might differ from desired descriptor based on render target writer's capabilities
		MikanRenderTargetDescriptor actualDescriptor;
		result = Mikan_GetRenderTargetDescriptor(context, &actualDescriptor);
		if (result == MikanResult_Success)
		{
			return m_requestManager->sendRequest(allocateRequest);
		}
	}

	return m_requestManager->addResponseHandler(INVALID_MIKAN_ID, MikanResult_SharedTextureError);
}

MikanResponseFuture MikanRenderTargetAPI::writeColorRenderTargetTexture(
	const MikanRequest& request)
{
	auto& writeRequest = static_cast<const WriteColorRenderTargetTexture&>(request);
	void* apiColorTexturePtr= writeRequest.apiColorTexturePtr;

	MikanContext context = m_requestManager->getContext();
	MikanResult result= Mikan_WriteColorRenderTargetTexture(context, apiColorTexturePtr);

	return m_requestManager->makeImmediateResponse(result);
}

MikanResponseFuture MikanRenderTargetAPI::writeDepthRenderTargetTexture(
	const MikanRequest& request)
{
	auto& writeRequest = static_cast<const WriteDepthRenderTargetTexture&>(request);
	void* apiDepthTexturePtr = writeRequest.apiDepthTexturePtr;
	float zNear= writeRequest.zNear;
	float zFar= writeRequest.zFar;

	MikanContext context = m_requestManager->getContext();
	MikanResult result = Mikan_WriteDepthRenderTargetTexture(context, apiDepthTexturePtr, zNear, zFar);

	return m_requestManager->makeImmediateResponse(result);
}

MikanResponseFuture MikanRenderTargetAPI::publishRenderTargetTextures(
	const MikanRequest& request)
{
	auto& publishRequest = static_cast<const PublishRenderTargetTextures&>(request);

	return m_requestManager->sendRequest(publishRequest);
}

MikanResponseFuture MikanRenderTargetAPI::freeRenderTargetTextures(
	const MikanRequest& request)
{
	auto& freeRequest = static_cast<const FreeRenderTargetTextures&>(request);

	// Free any locally allocated resources
	MikanContext context = m_requestManager->getContext();
	Mikan_FreeRenderTargetTextures(context);

	// Tell the server to free the render target resources too
	return m_requestManager->sendRequest(freeRequest);
}