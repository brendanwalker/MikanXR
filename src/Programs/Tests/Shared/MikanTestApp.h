#pragma once

#include "MikanAPI.h"
#include "MikanAnchorTypes.h"
#include "MikanCameraTypes.h"
#include "MikanCameraRequests.h"
#include "MikanCameraRenderTarget.h"
#include "MikanCompositorTypes.h"
#include "MikanMarkerTypes.h"
#include "MikanClientRequests.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "MikanPropertyRequests.h"
#include "MikanPropertyEvents.h"
#include "MikanPropertyTypes.h"
#include "MikanSceneTypes.h"
#include "MikanStageTypes.h"
#include "MikanStencilRequests.h"
#include "MikanTextureSourceTypes.h"
#include "MikanTrackingMountTypes.h"
#include "MikanTrackingVolumeTypes.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"
#include "MikanMathTypes.h"

#include "Logger.h"

#include <chrono>
#include <memory>
#include <map>

class MikanTestApp
{
public:
	MikanTestApp();
	virtual ~MikanTestApp();

	int exec(const char* szClientName, int argc, char** argv);

	inline void requestShutdown(int retCode)
	{
		m_bShutdownRequested = true;
		m_returnCode= retCode;
	}

	// Log Helpers
	static void logComponent(const MikanComponentValues& componentInfo);
	static void logComponent(const MikanTransformComponentValues& transformInfo);
	static void logComponent(const MikanAnchorComponentValues& anchorInfo);
	static void logComponent(const MikanStencilComponentValues& stencilInfo);
	static void logComponent(const MikanQuadStencilComponentValues& quadInfo);
	static void logComponent(const MikanBoxStencilComponentValues& boxInfo);
	static void logComponent(const MikanModelStencilComponentValues& modelInfo);
	static void logComponent(const MikanVRDeviceComponentValues& vrDeviceInfo);
	static void logModelStencilGeometry(const MikanStencilModelRenderGeometry& geometry);
	static void logModelTriMesh(const MikanTriagulatedMesh& triMesh);
	static void logTransform(const MikanTransform& xform);

protected:
	virtual MikanClientGraphicsApi getGraphicsApi() const = 0;

	static void onMikanLog(int log_level, const char* log_message);

	virtual bool startup(const char* szClientName, int argc, char** argv);
	virtual void update(const double deltaSeconds);
	virtual bool renderToCameraTarget(MikanCameraRenderTarget* cameraRenderTarget) const = 0;
	virtual bool renderToViewport() const = 0;
	virtual void shutdown();

	// Camera Render Target Helpers
	virtual MikanCameraRenderTargetPtr allocateCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId) = 0;
	MikanCameraRenderTargetPtr getOrAddCameraRenderTarget(MikanCameraID cameraId);
	void disposeCameraRenderTarget(MikanCameraID cameraId);

	// App Connection Events
	virtual void handleMikanConnected();
	virtual void handleMikanDisconnected(const MikanDisconnectedEvent& disconnectEvent);
	virtual void handleNewVideoSourceFrame(const MikanCameraNewFrameEvent& newFrameEvent);

	// Component Events
	virtual void handleMikanEvent(MikanEventPtr mikanEvent);

	// Property Change Events
	virtual void handlePropertyUpdateEvent(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleComponentPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleComponentNameChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformScaleChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformOrientationChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleTransformPositionChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleVRDevicePropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleAnchorPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleBoxStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleQuadStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);
	virtual void handleModelStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent);

protected:
	IMikanAPIPtr m_mikanApi;
	bool m_mikanInitialized = false;
	MikanCameraID m_lastProcessedCamera= INVALID_MIKAN_ID;
	int64_t m_lastReceivedVideoSourceFrame = 0;

	// Mapping of cameras to render targets
	std::map<MikanCameraID, MikanCameraRenderTargetPtr> m_cameraRenderTargetMap;

	// Flag requesting that we exit the update loop
	bool m_bShutdownRequested = false;
	int m_returnCode= 0;

	// Current FPS rate
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTimestamp;
	float m_fps = 0.f;
};