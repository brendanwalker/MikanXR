#pragma once

#include "TestGraphicsContext.h"
#include "MikanTypeFwd.h"

#include <functional>
#include <memory>
#include <map>

class TestMikanClient
{
public:
	TestMikanClient(TestGraphicsContext* graphicsContext);
	virtual ~TestMikanClient();

	inline bool getIsShutdownRequested() const { return m_bShutdownRequested; }

	bool init(const char* szClientName);
	void update(const float deltaSeconds);
	void dispose();

protected:
	static void onMikanLog(int log_level, const char* log_message);

	// App Connection Events
	void handleMikanConnected();
	void handleMikanDisconnected(const struct MikanDisconnectedEvent& disconnectEvent);
	void handleCameraNewFrameEvent(const struct MikanCameraNewFrameEvent& newFrameEvent);

	// Component Events
	void handleMikanEvent(MikanEventPtr mikanEvent);

	// Property Change Events
	void handlePropertyUpdateEvent(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleComponentPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleComponentNameChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleTransformPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleTransformScaleChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleTransformOrientationChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleTransformPositionChanged(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleVRDevicePropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleAnchorPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleBoxStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleQuadStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);
	void handleModelStencilPropertyUpdate(const struct MikanPropertyUpdateEvent& propertyUpdateEvent);

	// Utility
	void makeFakeCameraNewFrameEvent(struct MikanCameraNewFrameEvent& fakeNewFrameEvent);

protected:
	TestGraphicsContext* m_graphicsContext= nullptr;
	IMikanAPIPtr m_mikanApi;
	bool m_mikanInitialized = false;
	MikanCameraID m_lastProcessedCamera= INVALID_MIKAN_ID;
	bool m_bShutdownRequested= false;
	float m_mikanReconnectTimout = 0.0f;
};

using TestMikanClientPtr = std::shared_ptr<TestMikanClient>;