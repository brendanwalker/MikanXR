#pragma once

#include "MikanCoreTypes.h"

#include <memory>

using IMikanAPIPtr = std::shared_ptr<class IMikanAPI>;
using TestCameraRenderTargetPtr = std::shared_ptr<class TestCameraRenderTarget>;

class ITestGraphicsContext
{
public:
	ITestGraphicsContext() = default;
	virtual ~ITestGraphicsContext() {}

	virtual MikanClientGraphicsApi getGraphicsApi() const = 0;
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId) = 0;
	virtual bool create(int windowWidth, int windowHeight) = 0;
	virtual void render() const = 0;
	virtual void dispose() = 0;
};