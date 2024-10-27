
#include "MikanRenderTargetAPI.h"
#include "MikanRequestManager.h"
#include "MikanCoreCAPI.h"

MikanRenderTargetAPI::MikanRenderTargetAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{}

MikanResult MikanRenderTargetAPI::setGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void* graphicsDeviceInterface)
{
	return Mikan_SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
}

MikanResult MikanRenderTargetAPI::getGraphicsDeviceInterface(
	MikanClientGraphicsApi api,
	void** outGraphicsDeviceInterface)
{
	return Mikan_GetGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
}

MikanResponseFuture MikanRenderTargetAPI::allocateRenderTargetTextures(
	const MikanRenderTargetDescriptor& descriptor)
{
	MikanRequestID requestId = INVALID_MIKAN_ID;
	MikanResult result = Mikan_AllocateRenderTargetTextures(&descriptor, &requestId);

	return m_requestManager->addResponseHandler(requestId, result);
}

MikanResult MikanRenderTargetAPI::writeColorRenderTargetTexture(void* apiColorTexturePtr)
{
	return Mikan_WriteColorRenderTargetTexture(apiColorTexturePtr);
}

MikanResult MikanRenderTargetAPI::writeDepthRenderTargetTexture(
	void* apiDepthTexturePtr,
	float zNear, 
	float zFar)
{
	return Mikan_WriteDepthRenderTargetTexture(apiDepthTexturePtr, zNear, zFar);
}

MikanResult MikanRenderTargetAPI::publishRenderTargetTextures(
	MikanClientFrameRendered& frameInfo)
{
	return Mikan_PublishRenderTargetTextures(&frameInfo);
}

MikanResponseFuture MikanRenderTargetAPI::freeRenderTargetTextures()
{
	MikanRequestID requestId = INVALID_MIKAN_ID;
	MikanResult result = Mikan_FreeRenderTargetTextures(&requestId);

	return m_requestManager->addResponseHandler(requestId, result);
}