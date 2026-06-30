
#include "MikanRenderTargetAPI.h"
#include "MikanRequestManager.h"
#include "MikanCameraRequests.h"
#include "MikanCoreCAPI.h"

MikanRenderTargetAPI::MikanRenderTargetAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
}

MikanAPIResult MikanRenderTargetAPI::setGraphicsDeviceInterface(MikanClientGraphicsApi api,
																void* graphicsDeviceInterface)
{
	MikanContext context= m_requestManager->getContext();

	return (MikanAPIResult)Mikan_SetGraphicsDeviceInterface(context, api, graphicsDeviceInterface);
}

MikanAPIResult MikanRenderTargetAPI::getGraphicsDeviceInterface(MikanClientGraphicsApi api,
																void** outGraphicsDeviceInterface)
{
	MikanContext context= m_requestManager->getContext();

	return (MikanAPIResult)Mikan_GetGraphicsDeviceInterface(context, api, outGraphicsDeviceInterface);
}

MikanAPIResult MikanRenderTargetAPI::setGraphicsCommandQueueInterface(MikanClientGraphicsApi api,
																	  void* graphicsCommandQueueInterface)
{
	MikanContext context= m_requestManager->getContext();

	return (MikanAPIResult)Mikan_SetGraphicsCommandQueueInterface(context, api, graphicsCommandQueueInterface);
}

MikanAPIResult MikanRenderTargetAPI::getGraphicsCommandQueueInterface(MikanClientGraphicsApi api,
																	  void** outGraphicsCommandQueueInterface)
{
	MikanContext context= m_requestManager->getContext();

	return (MikanAPIResult)Mikan_GetGraphicsCommandQueueInterface(context, api, outGraphicsCommandQueueInterface);
}

MikanAPIResult MikanRenderTargetAPI::getCameraPackDepthTextureResourcePtr(MikanCameraID cameraId, void** outResourcePtr)
{
	if (outResourcePtr != nullptr)
	{
		MikanContext context= m_requestManager->getContext();
		void* resourcePtr= Mikan_GetCameraPackDepthTextureResourcePtr(context, cameraId);

		if (resourcePtr != nullptr)
		{
			*outResourcePtr= resourcePtr;

			return MikanAPIResult::Success;
		}
	}

	return MikanAPIResult::RequestFailed;
}

MikanResponseFuture MikanRenderTargetAPI::tryProcessRequest(MikanRequest& request)
{
	if (typeid(request) == typeid(AllocateCameraRenderTargetTextures))
	{
		return allocateRenderTargetTextures(request);
	}
	else if (typeid(request) == typeid(WriteCameraColorRenderTargetTexture))
	{
		return writeColorRenderTargetTexture(request);
	}
	else if (typeid(request) == typeid(WriteCameraDepthRenderTargetTexture))
	{
		return writeDepthRenderTargetTexture(request);
	}
	else if (typeid(request) == typeid(WriteCameraShadowRenderTargetTexture))
	{
		return writeShadowRenderTargetTexture(request);
	}
	else if (typeid(request) == typeid(PublishCameraRenderTargetTextures))
	{
		return publishRenderTargetTextures(request);
	}
	else if (typeid(request) == typeid(FreeCameraRenderTargetTextures))
	{
		return freeRenderTargetTextures(request);
	}

	return MikanResponseFuture();
}

MikanResponseFuture MikanRenderTargetAPI::allocateRenderTargetTextures(MikanRequest& request)
{
	auto& allocateRequest= static_cast<AllocateCameraRenderTargetTextures&>(request);
	const MikanRenderTargetDescriptor& descriptor= allocateRequest.descriptor;

	MikanContext context= m_requestManager->getContext();

	// Create the shared texture
	MikanAPIResult result=
		(MikanAPIResult)Mikan_AllocateCameraRenderTargetTextures(context, allocateRequest.camera_id, &descriptor);
	if (result == MikanAPIResult::Success)
	{
		// Actual descriptor might differ from desired descriptor based on render target writer's capabilities
		MikanRenderTargetDescriptor actualDescriptor;
		result= (MikanAPIResult)Mikan_GetCameraRenderTargetDescriptor(context, allocateRequest.camera_id,
																	  &actualDescriptor);
		if (result == MikanAPIResult::Success)
		{
			// Overwrite the descriptor in the original request with the actual descriptor
			allocateRequest.descriptor= actualDescriptor;

			return m_requestManager->sendRequest(allocateRequest);
		}
	}

	return m_requestManager->addResponseHandler(INVALID_MIKAN_ID, MikanAPIResult::RequestFailed);
}

MikanResponseFuture MikanRenderTargetAPI::writeColorRenderTargetTexture(MikanRequest& request)
{
	auto& writeRequest= static_cast<WriteCameraColorRenderTargetTexture&>(request);
	void* apiColorTexturePtr= writeRequest.api_color_texture_ptr;

	MikanContext context= m_requestManager->getContext();
	MikanAPIResult result=
		(MikanAPIResult)Mikan_WriteCameraColorRenderTargetTexture(context, writeRequest.camera_id, apiColorTexturePtr);

	return MikanResponseFuture(result);
}

MikanResponseFuture MikanRenderTargetAPI::writeDepthRenderTargetTexture(MikanRequest& request)
{
	auto& writeRequest= static_cast<WriteCameraDepthRenderTargetTexture&>(request);
	void* apiDepthTexturePtr= writeRequest.api_depth_texture_ptr;
	float zNear= writeRequest.z_near;
	float zFar= writeRequest.z_far;

	MikanContext context= m_requestManager->getContext();
	MikanAPIResult result= (MikanAPIResult)Mikan_WriteCameraDepthRenderTargetTexture(context, writeRequest.camera_id,
																					 apiDepthTexturePtr, zNear, zFar);

	return MikanResponseFuture(result);
}

MikanResponseFuture MikanRenderTargetAPI::writeShadowRenderTargetTexture(MikanRequest& request)
{
	auto& writeRequest= static_cast<WriteCameraShadowRenderTargetTexture&>(request);
	void* apiShadowTexturePtr= writeRequest.api_shadow_texture_ptr;

	MikanContext context= m_requestManager->getContext();
	MikanAPIResult result= (MikanAPIResult)Mikan_WriteCameraShadowRenderTargetTexture(
		context, writeRequest.camera_id, apiShadowTexturePtr);

	return MikanResponseFuture(result);
}

MikanResponseFuture MikanRenderTargetAPI::publishRenderTargetTextures(MikanRequest& request)
{
	auto& publishRequest= static_cast<PublishCameraRenderTargetTextures&>(request);

	return m_requestManager->sendRequest(publishRequest);
}

MikanResponseFuture MikanRenderTargetAPI::freeRenderTargetTextures(MikanRequest& request)
{
	auto& freeRequest= static_cast<FreeCameraRenderTargetTextures&>(request);

	// Free any locally allocated resources
	MikanContext context= m_requestManager->getContext();
	Mikan_FreeCameraRenderTargetTextures(context, freeRequest.camera_id);

	// Tell the server to free the render target resources too
	return m_requestManager->sendRequest(freeRequest);
}