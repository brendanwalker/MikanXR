#pragma once

#include "MikanAPI.h"

class MikanCameraRenderTarget
{
public:
	MikanCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId);
	virtual ~MikanCameraRenderTarget();
	
	using RenderCallback= std::function<bool(MikanCameraRenderTarget*)>;
	bool processCameraNewFrameEvent(const struct MikanCameraNewFrameEvent& newFrameEvent, RenderCallback renderCallback);
	void dispose();

protected:
	bool reallocateRenderTarget(int textureWidth, int textureHeight);
	
	bool createSharedTexture(int textureWidth, int textureHeight);
	void freeSharedTexture();

	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) = 0;
	virtual void freeGraphicsAPIResources() = 0;
	virtual void* getGraphicsApiColorTexturePtr() const = 0;
	virtual void* getGraphicsApiDepthTexturePtr() const = 0;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) = 0;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) = 0;

protected:
	IMikanAPIWeakPtr m_mikanAPI;
	MikanCameraID m_cameraId = INVALID_MIKAN_ID;
	
	int m_width = 0;
	int m_height = 0;
	bool m_bHasAllocatedRemoteTexture = false;
	bool m_bHasValidProjMatrix= false;
	bool m_bHasValidViewMatrix= false;

	int64_t m_lastReceivedFrameIndex= 0;
};

using MikanCameraRenderTargetPtr = std::shared_ptr<MikanCameraRenderTarget>;