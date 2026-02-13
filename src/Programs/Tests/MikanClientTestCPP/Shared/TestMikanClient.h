#pragma once

#include "ITestGraphicsContext.h"
#include "MikanTypeFwd.h"

#include <functional>
#include <memory>
#include <map>

class TestMikanClient
{
public:
	TestMikanClient(ITestGraphicsContext* graphicsContext);
	virtual ~TestMikanClient();

	inline bool getIsShutdownRequested() const { return m_bShutdownRequested; }

	using RenderCallback = std::function<bool(class TestCameraRenderTarget*)>;
	inline void setRenderCallback(RenderCallback renderCallback) { m_renderCallback= renderCallback; }

	virtual bool init(const char* szClientName);
	virtual void update(const double deltaSeconds);
	virtual void dispose();

protected:
	virtual MikanClientGraphicsApi getGraphicsApi() const = 0;

	static void onMikanLog(int log_level, const char* log_message);

	// Camera Render Target Helpers
	TestCameraRenderTargetPtr getOrAddCameraRenderTarget(MikanCameraID cameraId);
	void disposeCameraRenderTarget(MikanCameraID cameraId);

	// App Connection Events
	virtual void handleMikanConnected();
	virtual void handleMikanDisconnected(const struct MikanDisconnectedEvent& disconnectEvent);
	virtual void handleNewVideoSourceFrame(const struct MikanCameraNewFrameEvent& newFrameEvent);

	// Component Events
	virtual void handleMikanEvent(MikanEventPtr mikanEvent);

	// Property Change Events
	virtual void handlePropertyUpdateEvent(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleComponentPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleComponentNameChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformScaleChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformOrientationChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformPositionChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleVRDevicePropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleAnchorPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleBoxStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleQuadStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleModelStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);

protected:
	ITestGraphicsContext* m_graphicsContext= nullptr;
	IMikanAPIPtr m_mikanApi;
	RenderCallback m_renderCallback;
	bool m_mikanInitialized = false;
	MikanCameraID m_lastProcessedCamera= INVALID_MIKAN_ID;
	int64_t m_lastReceivedVideoSourceFrame = 0;
	bool m_bShutdownRequested= false;

	// Mapping of cameras to render targets
	std::map<MikanCameraID, TestCameraRenderTargetPtr> m_cameraRenderTargetMap;
};