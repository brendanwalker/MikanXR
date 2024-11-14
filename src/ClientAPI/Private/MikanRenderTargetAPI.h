#pragma once

#include "MikanAPI.h"
#include <string>

class MikanRenderTargetAPI
{
public:
	MikanRenderTargetAPI() = default;
	MikanRenderTargetAPI(class MikanRequestManager* requestManager);

	MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface);
	MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface);
	bool tryProcessRequest(const MikanRequest& request, MikanResponseFuture& outFuture);

protected:
	MikanResponseFuture allocateRenderTargetTextures(const MikanRequest& request);
	MikanResponseFuture writeColorRenderTargetTexture(const MikanRequest& request);
	MikanResponseFuture writeDepthRenderTargetTexture(const MikanRequest& request);
	MikanResponseFuture publishRenderTargetTextures(const MikanRequest& request);
	MikanResponseFuture freeRenderTargetTextures(const MikanRequest& request);

private:
	class MikanRequestManager* m_requestManager = nullptr;
};