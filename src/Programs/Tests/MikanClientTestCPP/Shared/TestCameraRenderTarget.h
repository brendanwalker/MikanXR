#pragma once

#include "MikanAPI.h"

using TestGraphicsContextPtr = std::shared_ptr<class TestGraphicsContext>;
using ITestGraphicsContextWeakPtr = std::weak_ptr<class TestGraphicsContext>;

class TestCameraRenderTarget
{
public:
	TestCameraRenderTarget(TestGraphicsContextPtr ownerContext, int cameraId);
	virtual ~TestCameraRenderTarget();

	inline MikanCameraID getCameraId() const { return m_cameraId; }
	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }
	inline float getZNear() const { return m_zNear; }
	inline float getZFar() const { return m_zFar; }

	bool processCameraNewFrameEvent(IMikanAPIPtr mikanAPI, const struct MikanCameraNewFrameEvent& newFrameEvent);
	void dispose(IMikanAPIPtr mikanAPI);

protected:
	bool reallocateRenderTarget(IMikanAPIPtr mikanAPI, int textureWidth, int textureHeight);
	
	bool createSharedTexture(IMikanAPIPtr mikanAPI, int textureWidth, int textureHeight);
	void freeSharedTexture(IMikanAPIPtr mikanAPI);

	virtual bool createGraphicsAPIResources(int textureWidth, int textureHeight) = 0;
	virtual void bindGraphicsAPIResource() = 0;
	virtual void unbindGraphicsAPIResource() = 0;
	virtual void freeGraphicsAPIResources() = 0;
	virtual void* getGraphicsApiColorTexturePtr() const = 0;
	virtual void* getGraphicsApiDepthTexturePtr() const = 0;

	virtual void updateCameraViewMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) = 0;
	virtual void updateCameraProjectionMatrix(const struct MikanCameraNewFrameEvent& newFrameEvent) = 0;

protected:
	ITestGraphicsContextWeakPtr m_ownerContext;
	MikanCameraID m_cameraId = INVALID_MIKAN_ID;
	
	int m_width = 0;
	int m_height = 0;
	float m_zNear = 0.f;
	float m_zFar = 0.f;
	bool m_bHasAllocatedRemoteTexture = false;
	bool m_bHasValidProjMatrix= false;
	bool m_bHasValidViewMatrix= false;

	int64_t m_lastReceivedFrameIndex= 0;
};

using TestCameraRenderTargetPtr = std::shared_ptr<TestCameraRenderTarget>;