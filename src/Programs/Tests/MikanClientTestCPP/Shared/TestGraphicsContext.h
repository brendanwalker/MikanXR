#pragma once

#include "MikanCoreTypes.h"

#include <map>
#include <memory>

using IMikanAPIPtr = std::shared_ptr<class IMikanAPI>;
using TestCameraRenderTargetPtr = std::shared_ptr<class TestCameraRenderTarget>;
using TestGraphicsContextPtr = std::shared_ptr<class TestGraphicsContext>;

class TestGraphicsContext
{
public:
	TestGraphicsContext(class TestApp* ownerApp);
	virtual ~TestGraphicsContext() {}

	inline TestApp* getOwnerApp() const { return m_ownerApp; }

	// Camera Render Target Helpers
	TestCameraRenderTargetPtr getCameraRenderTarget(MikanCameraID cameraId) const;
	TestCameraRenderTargetPtr getOrAddCameraRenderTarget(IMikanAPIPtr mikanApi, MikanCameraID cameraId);
	void removeCameraRenderTarget(IMikanAPIPtr mikanApi, MikanCameraID cameraId);
	void removeAllCameraRenderTargets(IMikanAPIPtr mikanApi);

	// Graphics API Interface
	virtual MikanClientGraphicsApi getGraphicsApi() const = 0;
	virtual void* getGraphicsDeviceInterface() const = 0;
	virtual struct SDL_Window* getSDLWindow() const = 0;
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId) = 0;
	virtual bool create(int windowWidth, int windowHeight) = 0;
	virtual void recreateMainRenderTarget() = 0;
	virtual void renderMainTarget() const = 0;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget) = 0;
	virtual void dispose() = 0;

protected:
	TestApp* m_ownerApp = nullptr;

	// Mapping of cameras to render targets
	std::map<MikanCameraID, TestCameraRenderTargetPtr> m_cameraRenderTargetMap;
};