#pragma once

#include "MikanCoreTypes.h"
#include "MikanMathTypes.h"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

using IMikanAPIPtr= std::shared_ptr<class IMikanAPI>;
using TestCameraRenderTargetPtr= std::shared_ptr<class TestCameraRenderTarget>;
using TestGraphicsContextPtr= std::shared_ptr<class TestGraphicsContext>;

class TestGraphicsContext
{
public:
	TestGraphicsContext(class TestApp* ownerApp);
	virtual ~TestGraphicsContext() {}

	inline class TestApp* getOwnerApp() const { return m_ownerApp; }

	// Main Window Helpers
	MikanVector2i getWindowPixelSize() const;

	// Camera Render Target Helpers
	TestCameraRenderTargetPtr getCameraRenderTarget(MikanCameraID cameraId) const;
	TestCameraRenderTargetPtr getOrAddCameraRenderTarget(MikanCameraID cameraId);
	void removeCameraRenderTarget(IMikanAPIPtr mikanApi, MikanCameraID cameraId);
	void removeAllCameraRenderTargets(IMikanAPIPtr mikanApi);

	// Graphics API Interface
	virtual MikanClientGraphicsApi getGraphicsApi() const= 0;
	virtual void* getGraphicsDeviceInterface() const= 0;
	virtual struct SDL_Window* getSDLWindow() const= 0;
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(int cameraId)= 0;
	virtual bool create(int windowWidth, int windowHeight)= 0;
	virtual void recreateMainRenderTarget()= 0;
	virtual void renderMainTarget() const= 0;
	virtual bool renderToCameraTarget(class TestCameraRenderTarget* cameraRenderTarget)= 0;
	virtual void dispose()= 0;

	// Frame dump support: the camera most recently drawn by renderToCameraTarget, and a CPU copy of a
	// camera target's color pixels as tightly packed RGBA8, top row first. A backend that cannot read
	// back answers false.
	virtual MikanCameraID getLastRenderedCameraId() const { return INVALID_MIKAN_ID; }
	virtual bool readCameraTargetPixels(class TestCameraRenderTarget* cameraRenderTarget,
										std::vector<uint8_t>& outRgbaPixels, int& outWidth, int& outHeight)
	{
		return false;
	}

protected:
	class TestApp* m_ownerApp= nullptr;

	// Mapping of cameras to render targets
	std::map<MikanCameraID, TestCameraRenderTargetPtr> m_cameraRenderTargetMap;
};