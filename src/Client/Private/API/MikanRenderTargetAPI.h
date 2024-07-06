#pragma once

#include "MikanAPI.h"

class MikanRenderTargetAPI : public IMikanRenderTargetAPI
{
public:
	MikanRenderTargetAPI() = default;
	MikanRenderTargetAPI(class MikanRequestManager* requestManager);

	virtual MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) override;
	virtual MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) override;
	virtual MikanResponseFuture allocateRenderTargetTextures(const MikanRenderTargetDescriptor& descriptor) override;
	virtual MikanResult writeColorRenderTargetTexture(void* apiColorTexturePtr) override;
	virtual MikanResult writeDepthRenderTargetTexture(void* apiDepthTexturePtr, float zNear, float zFar) override;
	virtual MikanResult publishRenderTargetTextures(MikanClientFrameRendered& frameInfo) override;
	virtual MikanResponseFuture freeRenderTargetTextures() override;

private:
	class MikanRequestManager* m_requestManager = nullptr;
};