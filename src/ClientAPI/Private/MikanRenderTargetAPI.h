#pragma once

#include "MikanAPI.h"
#include <string>

class MikanRenderTargetAPI
{
public:
	MikanRenderTargetAPI() = default;
	MikanRenderTargetAPI(class MikanRequestManager* requestManager);

	MikanAPIResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface);
	MikanAPIResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface);
	bool tryProcessRequest(MikanRequest& request, MikanResponseFuture& outFuture);

protected:
	MikanResponseFuture allocateRenderTargetTextures(MikanRequest& request);
	MikanResponseFuture writeColorRenderTargetTexture(MikanRequest& request);
	MikanResponseFuture writeDepthRenderTargetTexture(MikanRequest& request);
	MikanResponseFuture publishRenderTargetTextures(MikanRequest& request);
	MikanResponseFuture freeRenderTargetTextures(MikanRequest& request);

private:
	class MikanRequestManager* m_requestManager = nullptr;
};