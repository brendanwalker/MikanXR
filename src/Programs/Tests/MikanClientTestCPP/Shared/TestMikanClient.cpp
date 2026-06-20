#include "TestMikanClient.h"
#include "TestLogUtils.h"

#include "MikanAPI.h"
#include "MikanAnchorTypes.h"
#include "MikanCameraTypes.h"
#include "MikanCameraRequests.h"
#include "TestCameraRenderTarget.h"
#include "MikanCompositorTypes.h"
#include "MikanMarkerTypes.h"
#include "MikanClientRequests.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanCameraEvents.h"
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

using namespace std::chrono_literals;

TestMikanClient::TestMikanClient(TestGraphicsContext* graphicsContext)
	: m_graphicsContext(graphicsContext)
	, m_mikanApi(IMikanAPI::createMikanAPI())
{
}

TestMikanClient::~TestMikanClient()
{
	dispose();
}

bool TestMikanClient::init(
	const char* szClientName)
{
	LoggerSettings settings= {};
	settings.min_log_level= LogSeverityLevel::info;
	settings.enable_console= true;

	log_init(settings);

	if (m_mikanApi->init(szClientName, MikanLogLevel_Info, onMikanLog) != MikanAPIResult::Success)
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize Mikan Client API";
		return false;
	}

	m_mikanInitialized= true;
	m_mikanApi->setGraphicsDeviceInterface(
		m_graphicsContext->getGraphicsApi(),
		m_graphicsContext->getGraphicsDeviceInterface());

	// Create a default camera render target to render to when we aren't connected to Mikan
	m_graphicsContext->getOrAddCameraRenderTarget(INVALID_MIKAN_ID);

	return true;
}

void TestMikanClient::dispose()
{
	// Clean up all render targets
	m_graphicsContext->removeAllCameraRenderTargets(m_mikanApi);

	// Shut down the mikan client connection
	if (m_mikanInitialized)
	{
		// If we are currently connected,
		// gracefully cleanup the client info on the server first
		if (m_mikanApi->getIsConnected())
		{
			DisposeClientRequest disposeRequest= {};
			m_mikanApi->sendRequest(disposeRequest).awaitResponse();
		}

		// Disconnect from the server and shutdown the API
		m_mikanApi->shutdown();
		m_mikanInitialized= false;
	}

	log_dispose();
}

void TestMikanClient::onMikanLog(int log_level, const char* log_message)
{
	switch (log_level)
	{
	case MikanLogLevel_Debug:
		MIKAN_LOG_DEBUG("onMikanLog") << log_message;
		break;
	case MikanLogLevel_Info:
		MIKAN_LOG_INFO("onMikanLog") << log_message;
		break;
	case MikanLogLevel_Warning:
		MIKAN_LOG_WARNING("onMikanLog") << log_message;
		break;
	case MikanLogLevel_Error:
		MIKAN_LOG_ERROR("onMikanLog") << log_message;
		break;
	case MikanLogLevel_Fatal:
		MIKAN_LOG_FATAL("onMikanLog") << log_message;
		break;
	default:
		MIKAN_LOG_INFO("onMikanLog") << log_message;
		break;
	}
}

void TestMikanClient::update(const float deltaSeconds)
{
	if (m_mikanApi->getIsConnected())
	{
		MikanEventPtr mikanEvent;
		while (m_mikanApi->fetchNextEvent(mikanEvent) == MikanAPIResult::Success)
		{
			// App Connection Events
			if (typeid(*mikanEvent) == typeid(MikanConnectedEvent))
			{
				handleMikanConnected();
			}
			else if (typeid(*mikanEvent) == typeid(MikanDisconnectedEvent))
			{
				auto disconnectEvent= std::static_pointer_cast<MikanDisconnectedEvent>(mikanEvent);

				handleMikanDisconnected(*disconnectEvent);
			}
			// Camera Frame Event
			else if (typeid(*mikanEvent) == typeid(MikanCameraNewFrameEvent))
			{
				auto newFrameEvent= std::static_pointer_cast<MikanCameraNewFrameEvent>(mikanEvent);

				handleCameraNewFrameEvent(*newFrameEvent);
			}
			// Generic Property Events
			else if (typeid(*mikanEvent) == typeid(MikanPropertyUpdateEvent))
			{
				auto propertyUpdateEvent= std::static_pointer_cast<MikanPropertyUpdateEvent>(mikanEvent);

				handlePropertyUpdateEvent(*propertyUpdateEvent);
			}
			// All other events
			else
			{
				handleMikanEvent(mikanEvent);
			}
		}
	}
	else
	{
		// Try to reconnect after timeout
		if (m_mikanReconnectTimout <= 0.0f)
		{
			if (m_mikanApi->connect() != MikanAPIResult::Success || !m_mikanApi->getIsConnected())
			{
				m_mikanReconnectTimout= 1.0f;
			}
		}
		else
		{
			m_mikanReconnectTimout-= deltaSeconds;
		}
	}

	// If we aren't connected to Mikan,
	// send a fake camera frame event with default values to render the default camera target
	if (!m_mikanApi->getIsConnected())
	{
		MikanCameraNewFrameEvent fakeNewFrameEvent= {};

		makeFakeCameraNewFrameEvent(fakeNewFrameEvent);
		handleCameraNewFrameEvent(fakeNewFrameEvent);
	}
}

// Component Events
template <class t_component_values>
static void handleComponentListChanged(IMikanAPIPtr mikanApi)
{
	// Fetch the list of anchors from Mikan and apply them to the scene
	GetComponentListRequest listRequest;
	listRequest.ownerSystem.setValue(t_component_values::k_ownerSystemName);
	listRequest.componentClassName.setValue(t_component_values::k_componentClassName);

	auto listResponse= mikanApi->sendRequest(listRequest).fetchResponse();
	if (listResponse->resultCode == MikanAPIResult::Success)
	{
		auto componentList= std::static_pointer_cast<ComponentListResponse>(listResponse);
		size_t componentCount= componentList->componentIdList.size();

		MIKAN_LOG_INFO("handleComponentListChanged") << t_component_values::k_componentClassName << " Count: " << componentCount;

		for (size_t Index= 0; Index < componentCount; ++Index)
		{
			const MikanComponentID componentId= componentList->componentIdList[Index];

			// TODO: Handle component creation and deletion
			ComponentGetValuesRequest componentRequest;
			componentRequest.ownerSystem.setValue(t_component_values::k_ownerSystemName);
			componentRequest.componentId= componentId;

			auto response= mikanApi->sendRequest(componentRequest).fetchResponse();
			if (response->resultCode == MikanAPIResult::Success)
			{
				auto componentValuesResponse=
					std::static_pointer_cast<ComponentGetValuesResponse>(response);
				const auto* typedComponentValues=
					componentValuesResponse->valuesObject.getTypedPointer<t_component_values>();

				TestLogUtils::logComponent(*typedComponentValues);
			}
		}
	}
}

// App Connection Events
void TestMikanClient::handleMikanConnected()
{
	// Initialize the client info on the server
	MikanClientInfo clientInfo= m_mikanApi->allocateClientInfo();
	clientInfo.engineName= "MikanXR Test";
	clientInfo.engineVersion= "1.0";
	clientInfo.applicationName= "MikanXR Client Test C++";
	clientInfo.applicationVersion= "1.0";
	clientInfo.graphicsAPI= m_graphicsContext->getGraphicsApi();
	clientInfo.supportsRGBA32= true;
	clientInfo.supportsDepth= true;

	InitClientRequest initClientRequest= {};
	initClientRequest.clientInfo= clientInfo;

	m_mikanApi->sendRequest(initClientRequest).awaitResponse();

	// Fetch all Mikan Components
	handleComponentListChanged<MikanAnchorComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanCameraComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanCompositorComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanMarkerComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanSceneComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanStageComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanQuadStencilComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanBoxStencilComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanModelStencilComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanTrackingMountComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanMarkerTrackingVolumeComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanVRTrackingVolumeComponentValues>(m_mikanApi);
	handleComponentListChanged<MikanClientTextureSourceValues>(m_mikanApi);
	handleComponentListChanged<MikanSpoutTextureSourceValues>(m_mikanApi);
	handleComponentListChanged<MikanNetworkVideoSourceValues>(m_mikanApi);
	handleComponentListChanged<MikanUSBVideoSourceValues>(m_mikanApi);
	handleComponentListChanged<MikanVRDeviceComponentValues>(m_mikanApi);
}

void TestMikanClient::handleMikanDisconnected(const MikanDisconnectedEvent& disconnectEvent)
{
	MIKAN_LOG_INFO("MikanDisconnectedEvent") << disconnectEvent.reason.getValue();

	if (disconnectEvent.code == MikanDisconnectCode_IncompatibleVersion)
	{
		// The server has disconnected us because we are using an incompatible version
		// Shutdown since connection is never going to work
		m_bShutdownRequested= true;
		MIKAN_LOG_ERROR("MikanDisconnectedEvent") << "Shutting down due to incompatible client";
	}
}

constexpr double degToRad(double degrees)
{
	return degrees * (3.14159265358979323846 / 180.0);
}

void TestMikanClient::makeFakeCameraNewFrameEvent(MikanCameraNewFrameEvent& fakeNewFrameEvent)
{
	static const float kDefaultHFov= 60.0f;
	static const float kDefaultZNear= 0.1f;
	static const float kDefaultZFar= 20.0f;

	MikanVector2i windowSize= m_graphicsContext->getWindowPixelSize();

	double aspectRatio= (double)windowSize.y / (double)windowSize.x;
	double c_x= (double)windowSize.x / 2.0;
	double c_y= (double)windowSize.y / 2.0;
	double hfov= kDefaultHFov;
	double vfov= hfov * aspectRatio;
	double f_x= c_x / tan(degToRad(hfov / 2.0));
	double f_y= c_y / tan(degToRad(vfov / 2.0));

	fakeNewFrameEvent.camera_id= INVALID_MIKAN_ID;
	fakeNewFrameEvent.camera_forward= {0.f, 0.f, -1.f};
	fakeNewFrameEvent.camera_up= {0.f, 1.f, 0.f};
	fakeNewFrameEvent.camera_position= {0.f, 0.f, 0.f};
	fakeNewFrameEvent.pixel_size= windowSize;
	fakeNewFrameEvent.focal_length= {f_x, f_y};
	fakeNewFrameEvent.principal_point= {c_x, c_y};
	fakeNewFrameEvent.z_bounds= {kDefaultZNear, kDefaultZFar};
	fakeNewFrameEvent.frame= 0;
}

void TestMikanClient::handleCameraNewFrameEvent(const MikanCameraNewFrameEvent& newFrameEvent)
{
	TestCameraRenderTargetPtr renderTarget=
		m_graphicsContext->getOrAddCameraRenderTarget(newFrameEvent.camera_id);

	if (renderTarget)
	{
		bool bProcessedFrame= renderTarget->processCameraNewFrameEvent(m_mikanApi, newFrameEvent);

		if (bProcessedFrame)
		{
			m_lastProcessedCamera= newFrameEvent.camera_id;
		}
	}
}

void TestMikanClient::handleMikanEvent(MikanEventPtr mikanEvent)
{
	MIKAN_LOG_INFO("handleMikanEvent") << "Received Event: " << mikanEvent->eventTypeName.getValue();
}

// Generic Property Events
void TestMikanClient::handlePropertyUpdateEvent(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const MikanPropertyValue& propertyValue= propertyUpdateEvent.propertyValue;
	const char* systemName= propertyValue.ownerSystem.getValue();
	const char* fieldName= propertyValue.fieldName.getValue();

	if (propertyValue.componentId != INVALID_MIKAN_ID)
	{
		const char* componentClass= propertyValue.ownerComponentClass.getValue();
		const char* componentName= propertyValue.fieldValue.getUtf8StringPointerValue();

		MIKAN_LOG_INFO("handlePropertyUpdateEvent")
			<< "Component Field Changed: "
			<< "systemClass: " << systemName
			<< ", componentClass: " << componentClass
			<< ", componentName: " << componentName
			<< ", fieldName: " << fieldName;
	}
	else
	{
		MIKAN_LOG_INFO("handlePropertyUpdateEvent")
			<< "System Field Changed: "
			<< "systemClass: " << systemName
			<< ", fieldName: " << fieldName;
	}

	if (systemName == MikanAnchorComponentValues::k_ownerSystemName)
	{
		handleAnchorPropertyUpdate(propertyUpdateEvent);
	}
	else if (systemName == MikanBoxStencilComponentValues::k_ownerSystemName)
	{
		handleBoxStencilPropertyUpdate(propertyUpdateEvent);
	}
	else if (systemName == MikanModelStencilComponentValues::k_ownerSystemName)
	{
		handleModelStencilPropertyUpdate(propertyUpdateEvent);
	}
	else if (systemName == MikanQuadStencilComponentValues::k_ownerSystemName)
	{
		handleQuadStencilPropertyUpdate(propertyUpdateEvent);
	}
	else if (systemName == MikanVRDeviceComponentValues::k_ownerSystemName)
	{
		handleVRDevicePropertyUpdate(propertyUpdateEvent);
	}
}

void TestMikanClient::handleComponentPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "component_name")
	{
		handleComponentNameChanged(propertyUpdateEvent);
	}
}

void TestMikanClient::handleComponentNameChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const char* componentClass= propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const char* componentName= propertyUpdateEvent.propertyValue.fieldValue.getUtf8StringPointerValue();

	MIKAN_LOG_INFO("HandleComponentNameChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Name Change: " << componentName;
}

// Transform Component Events
void TestMikanClient::handleTransformPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "relative_scale")
	{
		handleTransformScaleChanged(propertyUpdateEvent);
	}
	else if (propertyUpdateEvent.propertyValue.fieldName == "relative_quaternion")
	{
		handleTransformOrientationChanged(propertyUpdateEvent);
	}
	else if (propertyUpdateEvent.propertyValue.fieldName == "relative_position")
	{
		handleTransformPositionChanged(propertyUpdateEvent);
	}
	else
	{
		handleComponentPropertyUpdate(propertyUpdateEvent);
	}
}

void TestMikanClient::handleTransformScaleChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const char* componentClass= propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const char* componentName= propertyUpdateEvent.propertyValue.fieldValue.getUtf8StringPointerValue();
	const MikanVector3f& s= propertyUpdateEvent.propertyValue.fieldValue.getVector3fValue();

	MIKAN_LOG_INFO("handleTransformScaleChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Scale Change: " << s.x << ", " << s.y << ", " << s.z;
}

void TestMikanClient::handleTransformOrientationChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const char* componentClass= propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const char* componentName= propertyUpdateEvent.propertyValue.fieldValue.getUtf8StringPointerValue();
	const MikanQuatf& q= propertyUpdateEvent.propertyValue.fieldValue.getQuaternionfValue();

	MIKAN_LOG_INFO("handleTransformOrientationChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Orientation Change: " << q.w << ", " << q.x << ", " << q.y << ", " << q.z;
}

void TestMikanClient::handleTransformPositionChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const char* componentClass= propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const char* componentName= propertyUpdateEvent.propertyValue.fieldValue.getUtf8StringPointerValue();
	const MikanVector3f& v= propertyUpdateEvent.propertyValue.fieldValue.getVector3fValue();

	MIKAN_LOG_INFO("handleTransformPositionChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Position Change: " << v.x << ", " << v.y << ", " << v.z;
}

// VR Device Events
void TestMikanClient::handleVRDevicePropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "VRDeviceComponentIdList")
	{
		handleComponentListChanged<MikanVRDeviceComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Anchor Events
void TestMikanClient::handleAnchorPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "AnchorComponentIdList")
	{
		handleComponentListChanged<MikanAnchorComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Box Stencil Events
void TestMikanClient::handleBoxStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "BoxStencilComponentIdList")
	{
		handleComponentListChanged<MikanBoxStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Quad Stencil Events
void TestMikanClient::handleQuadStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "QuadStencilComponentIdList")
	{
		handleComponentListChanged<MikanQuadStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Model Stencil Events
void TestMikanClient::handleModelStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName == "ModelStencilComponentIdList")
	{
		handleComponentListChanged<MikanModelStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}