#include "TestApp.h"
#include "TestGraphicsContext.h"
#include "TestCameraRenderTarget.h"

TestGraphicsContext::TestGraphicsContext(TestApp* ownerApp)
	: m_ownerApp(ownerApp)
{
}

TestCameraRenderTargetPtr TestGraphicsContext::getCameraRenderTarget(MikanCameraID cameraId) const
{
	auto it = m_cameraRenderTargetMap.find(cameraId);
	if (it != m_cameraRenderTargetMap.end())
	{
		return it->second;
	}

	return nullptr;
}

TestCameraRenderTargetPtr TestGraphicsContext::getOrAddCameraRenderTarget(
	IMikanAPIPtr mikanApi,
	MikanCameraID cameraId)
{
	TestCameraRenderTargetPtr renderTarget = getCameraRenderTarget(cameraId);
	if (!renderTarget)
	{
		renderTarget = allocateCameraRenderTarget(cameraId);
		m_cameraRenderTargetMap.insert({ cameraId, renderTarget });
	}

	return renderTarget;
}

void TestGraphicsContext::removeCameraRenderTarget(
	IMikanAPIPtr mikanApi, 
	MikanCameraID cameraId)
{
	auto it = m_cameraRenderTargetMap.find(cameraId);
	if (it != m_cameraRenderTargetMap.end())
	{
		TestCameraRenderTargetPtr cameraRenderTarget = it->second;

		// Teardown the render target 
		cameraRenderTarget->dispose(mikanApi);

		// Remove the entry from the map
		m_cameraRenderTargetMap.erase(it);
	}
}

void TestGraphicsContext::removeAllCameraRenderTargets(IMikanAPIPtr mikanApi)
{
	for (auto it : m_cameraRenderTargetMap)
	{
		TestCameraRenderTargetPtr cameraRenderTarget = it.second;
		
		// Signal to mikan to tear down shared texture on its end
		cameraRenderTarget->dispose(mikanApi);
	}

	m_cameraRenderTargetMap.clear();
}