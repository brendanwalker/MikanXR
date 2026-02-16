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

	virtual bool init(const char* szClientName);
	virtual void update(const float deltaSeconds);
	virtual void dispose();

protected:
	static void onMikanLog(int log_level, const char* log_message);

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
	TestGraphicsContext* m_graphicsContext= nullptr;
	IMikanAPIPtr m_mikanApi;
	bool m_mikanInitialized = false;
	MikanCameraID m_lastProcessedCamera= INVALID_MIKAN_ID;
	bool m_bShutdownRequested= false;
	float m_mikanReconnectTimout = 0.0f;
};

using TestMikanClientPtr = std::shared_ptr<TestMikanClient>;