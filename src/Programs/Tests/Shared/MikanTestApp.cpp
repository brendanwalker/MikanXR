#include "MikanTestApp.h"

using namespace std::chrono_literals;

MikanTestApp::MikanTestApp()
	: m_mikanApi(IMikanAPI::createMikanAPI())
{
}

MikanTestApp::~MikanTestApp()
{
	shutdown();
}

int MikanTestApp::exec(const char* szClientName, int argc, char** argv)
{
	if (startup(szClientName, argc, argv))
	{
		m_lastFrameTimestamp = std::chrono::high_resolution_clock::now();

		while (!m_bShutdownRequested)
		{
			// Update the frame rate
			auto now = std::chrono::high_resolution_clock::now();
			const double deltaSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_lastFrameTimestamp).count();
			m_fps = deltaSeconds > 0.0 ? (float)(1.0 / deltaSeconds) : 0.f;
			m_lastFrameTimestamp = now;

			update(deltaSeconds);
		}
	}
	else
	{
		MIKAN_LOG_ERROR("exec") << "Failed to initialize application!";
	}

	shutdown();

	return m_returnCode;
}

bool MikanTestApp::startup(const char* szClientName, int argc, char** argv)
{
	bool success = true;

	LoggerSettings settings = {};
	settings.min_log_level = LogSeverityLevel::info;
	settings.enable_console = true;

	log_init(settings);

	if (m_mikanApi->init(szClientName, MikanLogLevel_Info, onMikanLog) == MikanAPIResult::Success)
	{
		m_mikanInitialized= true;
	}
	else
	{
		MIKAN_LOG_ERROR("startup") << "Failed to initialize Mikan Client API";
		success= false;
	}

	return success;
}

void MikanTestApp::shutdown()
{
	// Clean up all render targets
	for (auto it : m_cameraRenderTargetMap)
	{
		// May signal to mikan to tear down shared texture on its end
		it.second->dispose();
	}
	m_cameraRenderTargetMap.clear();

	// Shut down the mikan client connection
	if (m_mikanInitialized)
	{
		// If we are currently connected, 
		// gracefully cleanup the client info on the server first
		if (m_mikanApi->getIsConnected())
		{
			DisposeClientRequest disposeRequest = {};
			m_mikanApi->sendRequest(disposeRequest).awaitResponse();
		}

		// Disconnect from the server and shutdown the API
		m_mikanApi->shutdown();
		m_mikanInitialized= false;
	}

	log_dispose();
}

void MikanTestApp::onMikanLog(int log_level, const char* log_message)
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

void MikanTestApp::update(const double deltaSeconds)
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
				auto disconnectEvent = std::static_pointer_cast<MikanDisconnectedEvent>(mikanEvent);

				handleMikanDisconnected(*disconnectEvent);
			}
			// Camera Frame Event
			else if (typeid(*mikanEvent) == typeid(MikanCameraNewFrameEvent))
			{
				auto newFrameEvent = std::static_pointer_cast<MikanCameraNewFrameEvent>(mikanEvent);

				handleNewVideoSourceFrame(*newFrameEvent);
			}
			// Generic Property Events
			else if (typeid(*mikanEvent) == typeid(MikanPropertyUpdateEvent))
			{
				auto propertyUpdateEvent = std::static_pointer_cast<MikanPropertyUpdateEvent>(mikanEvent);

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
		// Just render the scene using the last applied camera pose
		renderToViewport();

		if (m_mikanApi->connect() != MikanAPIResult::Success)
		{
			// timeout between reconnect attempts
			std::this_thread::sleep_for(1000ms);
		}
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

	auto listResponse = mikanApi->sendRequest(listRequest).fetchResponse();
	if (listResponse->resultCode == MikanAPIResult::Success)
	{
		auto componentList = std::static_pointer_cast<ComponentListResponse>(listResponse);
		size_t componentCount = componentList->componentIdList.size();

		MIKAN_LOG_INFO("handleComponentListChanged") <<
			t_component_values::k_componentClassName << " Count: " << componentCount;

		for (size_t Index = 0; Index < componentCount; ++Index)
		{
			const MikanComponentID componentId = componentList->componentIdList[Index];

			ComponentGetValuesRequest componentRequest;
			componentRequest.ownerSystem.setValue(t_component_values::k_ownerSystemName);
			componentRequest.componentId = componentId;

			auto response = mikanApi->sendRequest(componentRequest).fetchResponse();
			if (response->resultCode == MikanAPIResult::Success)
			{
				auto componentValuesResponse =
					std::static_pointer_cast<ComponentGetValuesResponse>(response);
				const auto* typedComponentValues =
					componentValuesResponse->valuesObject.getTypedPointer<t_component_values>();

				MikanTestApp::logComponent(*typedComponentValues);
			}
		}
	}
}

// Camera Render Target Helpers
MikanCameraRenderTargetPtr MikanTestApp::getOrAddCameraRenderTarget(MikanCameraID cameraId)
{
	auto it= m_cameraRenderTargetMap.find(cameraId);
	if (it != m_cameraRenderTargetMap.end())
	{
		return it->second;
	}

	MikanCameraRenderTargetPtr newCameraRenderTarget= allocateCameraRenderTarget(m_mikanApi, cameraId);
	m_cameraRenderTargetMap.insert({cameraId, newCameraRenderTarget});

	return newCameraRenderTarget;
}

void MikanTestApp::disposeCameraRenderTarget(MikanCameraID cameraId)
{
	auto it = m_cameraRenderTargetMap.find(cameraId);
	if (it != m_cameraRenderTargetMap.end())
	{
		// Teardown the render target 
		it->second->dispose();

		// Remove the entry from the map
		m_cameraRenderTargetMap.erase(it);
	}
}

// App Connection Events
void MikanTestApp::handleMikanConnected()
{
	// Initialize the client info on the server
	MikanClientInfo clientInfo = m_mikanApi->allocateClientInfo();
	clientInfo.supportsRGB24 = true;
	clientInfo.engineName = "MikanXR Test";
	clientInfo.engineVersion = "1.0";
	clientInfo.applicationName = "MikanXR Test";
	clientInfo.applicationVersion = "1.0";
	clientInfo.graphicsAPI = getGraphicsApi();

	InitClientRequest initClientRequest = {};
	initClientRequest.clientInfo = clientInfo;

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

void MikanTestApp::handleMikanDisconnected(const MikanDisconnectedEvent& disconnectEvent)
{
	MIKAN_LOG_INFO("MikanDisconnectedEvent") << disconnectEvent.reason.getValue();

	if (disconnectEvent.code == MikanDisconnectCode_IncompatibleVersion)
	{
		// The server has disconnected us because we are using an incompatible version
		// Shutdown since connection is never going to work
		m_bShutdownRequested = true;
		MIKAN_LOG_ERROR("MikanDisconnectedEvent") << "Shutting down due to incompatible client";
	}
}

void MikanTestApp::handleNewVideoSourceFrame(const MikanCameraNewFrameEvent& newFrameEvent)
{
	MikanCameraRenderTargetPtr renderTarget= getOrAddCameraRenderTarget(newFrameEvent.camera_id);

	if (renderTarget)
	{
		bool bProcessedFrame= renderTarget->processCameraNewFrameEvent(
			newFrameEvent,
			[this](MikanCameraRenderTarget* cameraRenderTarget) {
				return renderToCameraTarget(cameraRenderTarget);
			});

		if (bProcessedFrame)
		{
			m_lastProcessedCamera= newFrameEvent.camera_id;
			renderToViewport();
		}
	}
}

void MikanTestApp::handleMikanEvent(MikanEventPtr mikanEvent)
{
	MIKAN_LOG_INFO("handleMikanEvent") << "Received Event: " << mikanEvent->eventTypeName.getValue();
}

// Generic Property Events
void MikanTestApp::handlePropertyUpdateEvent(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const MikanPropertyValue& propertyValue = propertyUpdateEvent.propertyValue;
	const std::string& systemName = propertyValue.ownerSystem.getValue();
	const std::string& fieldName = propertyValue.fieldName.getValue();

	if (propertyValue.componentId != INVALID_MIKAN_ID)
	{
		const std::string& componentClass = propertyValue.ownerComponentClass.getValue();
		const std::string& componentName = propertyValue.fieldValue.getStringValue();

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

void MikanTestApp::handleComponentPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "component_name")
	{
		handleComponentNameChanged(propertyUpdateEvent);
	}
}

void MikanTestApp::handleComponentNameChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const std::string& componentClass = propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const std::string& componentName = propertyUpdateEvent.propertyValue.fieldValue.getStringValue();

	MIKAN_LOG_INFO("HandleComponentNameChanged") 
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Name Change: " << componentName;
}
	
// Transform Component Events
void MikanTestApp::handleTransformPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "relative_scale")
	{
		handleTransformScaleChanged(propertyUpdateEvent);
	}
	else if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "relative_rotation")
	{
		handleTransformOrientationChanged(propertyUpdateEvent);
	}
	else if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "relative_position")
	{
		handleTransformPositionChanged(propertyUpdateEvent);
	}
	else
	{
		handleComponentPropertyUpdate(propertyUpdateEvent);
	}
}

void MikanTestApp::handleTransformScaleChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const std::string& componentClass = propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const std::string& componentName = propertyUpdateEvent.propertyValue.fieldValue.getStringValue();
	const MikanVector3f& s = propertyUpdateEvent.propertyValue.fieldValue.getVector3fValue();

	MIKAN_LOG_INFO("handleTransformScaleChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Scale Change: " << s.x << ", " << s.y << ", " << s.z;
}

void MikanTestApp::handleTransformOrientationChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const std::string& componentClass = propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const std::string& componentName = propertyUpdateEvent.propertyValue.fieldValue.getStringValue();
	const MikanQuatf& q = propertyUpdateEvent.propertyValue.fieldValue.getQuaternionfValue();

	MIKAN_LOG_INFO("handleTransformOrientationChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Orientation Change: " << q.x << ", " << q.y << ", " << q.z << ", " << q.w;
}

void MikanTestApp::handleTransformPositionChanged(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	const std::string& componentClass = propertyUpdateEvent.propertyValue.ownerComponentClass.getValue();
	const std::string& componentName = propertyUpdateEvent.propertyValue.fieldValue.getStringValue();
	const MikanVector3f& v = propertyUpdateEvent.propertyValue.fieldValue.getVector3fValue();

	MIKAN_LOG_INFO("handleTransformPositionChanged")
		<< "Component(class: " << componentClass
		<< ", id: " << propertyUpdateEvent.propertyValue.componentId
		<< "), Position Change: " << v.x << ", " << v.y << ", " << v.z;
}

// VR Device Events
void MikanTestApp::handleVRDevicePropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "VRDeviceComponentIdList")
	{
		handleComponentListChanged<MikanVRDeviceComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Anchor Events
void MikanTestApp::handleAnchorPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "AnchorComponentIdList")
	{
		handleComponentListChanged<MikanAnchorComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Box Stencil Events
void MikanTestApp::handleBoxStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "BoxStencilComponentIdList")
	{
		handleComponentListChanged<MikanBoxStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Quad Stencil Events
void MikanTestApp::handleQuadStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "QuadStencilComponentIdList")
	{
		handleComponentListChanged<MikanQuadStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Model Stencil Events
void MikanTestApp::handleModelStencilPropertyUpdate(const MikanPropertyUpdateEvent& propertyUpdateEvent)
{
	if (propertyUpdateEvent.propertyValue.fieldName.getValue() == "ModelStencilComponentIdList")
	{
		handleComponentListChanged<MikanModelStencilComponentValues>(m_mikanApi);
	}
	else
	{
		handleTransformPropertyUpdate(propertyUpdateEvent);
	}
}

// Log Helpers
void MikanTestApp::logComponent(const MikanComponentValues& componentInfo)
{
	MIKAN_LOG_INFO("MikanComponent") << "Component ID: " << componentInfo.component_id;
	MIKAN_LOG_INFO("MikanComponent") << "Component Name: " << componentInfo.component_name.getValue();
}

void MikanTestApp::logComponent(const MikanTransformComponentValues& transformInfo)
{
	logComponent((const MikanComponentValues&)transformInfo);

	const MikanVector3f& s = transformInfo.relative_scale;
	const MikanQuatf& r = transformInfo.relative_rotation;
	const MikanVector3f& t = transformInfo.relative_position;

	MIKAN_LOG_INFO("logTransformInfo") << "Scale: " << s.x << ", " << s.y << ", " << s.z;
	MIKAN_LOG_INFO("logTransformInfo") << "Rotation: " << r.x << ", " << r.y << ", " << r.z << ", " << r.w;
	MIKAN_LOG_INFO("logTransformInfo") << "Position: " << t.x << ", " << t.y << ", " << t.z;
}

void MikanTestApp::logComponent(const MikanAnchorComponentValues& anchorInfo)
{
	logComponent((const MikanTransformComponentValues&)anchorInfo);
		
	MIKAN_LOG_INFO("AnchorComponent") << "Owner Stage Id: " << anchorInfo.stage_id;
}

void MikanTestApp::logComponent(const MikanStencilComponentValues& stencilInfo)
{
	logComponent((const MikanTransformComponentValues&)stencilInfo);

	MIKAN_LOG_INFO("StencilComponent") << "Parent Anchor Id: " << stencilInfo.parent_anchor_id;
	MIKAN_LOG_INFO("StencilComponent") << "Is Disabled: " << (stencilInfo.is_disabled ? "true" : "false");
	MIKAN_LOG_INFO("StencilComponent") << "Cull Mode: " << stencilInfo.cull_mode;
}

void MikanTestApp::logComponent(const MikanQuadStencilComponentValues& quadInfo)
{
	logComponent((const MikanStencilComponentValues&)quadInfo);

	MIKAN_LOG_INFO("QuadStencilComponent") << "Quad Width: " << quadInfo.quad_width;
	MIKAN_LOG_INFO("QuadStencilComponent") << "Quad Height: " << quadInfo.quad_height;
}

void MikanTestApp::logComponent(const MikanBoxStencilComponentValues& boxInfo)
{
	logComponent((const MikanStencilComponentValues&)boxInfo);

	MIKAN_LOG_INFO("BoxStencilComponent") << "Box X Size: " << boxInfo.box_x_size;
	MIKAN_LOG_INFO("BoxStencilComponent") << "Box Y Size: " << boxInfo.box_y_size;
	MIKAN_LOG_INFO("BoxStencilComponent") << "Box Z Size: " << boxInfo.box_z_size;
}

void MikanTestApp::logComponent(const MikanModelStencilComponentValues& modelInfo)
{
	logComponent((const MikanStencilComponentValues&)modelInfo);

	MIKAN_LOG_INFO("ModelStencilComponent") << "Model: " << modelInfo.model_path.getValue();
}

void MikanTestApp::logComponent(const MikanVRDeviceComponentValues& vrDeviceInfo)
{
	MIKAN_LOG_INFO("VRDeviceComponent") << "Device Name: " << vrDeviceInfo.vr_device_path.getValue();
}

void MikanTestApp::logModelStencilGeometry(const MikanStencilModelRenderGeometry& geometry)
{
	for (size_t index = 0; index < geometry.meshes.size(); ++index)
	{
		const MikanTriagulatedMesh& mesh = geometry.meshes[index];

		MIKAN_LOG_INFO("logModelStencilGeometry") << "  Mesh Index: " << index;
		logModelTriMesh(mesh);
	}
}

void MikanTestApp::logModelTriMesh(const MikanTriagulatedMesh& triMesh)
{
	MIKAN_LOG_INFO("logModelTriMesh") << "    Triangle Count: " << triMesh.indices.size() / 3;
	MIKAN_LOG_INFO("logModelTriMesh") << "    Normal Count: " << triMesh.normals.size();
	MIKAN_LOG_INFO("logModelTriMesh") << "    Vertex Count: " << triMesh.vertices.size();
	MIKAN_LOG_INFO("logModelTriMesh") << "    Texel Count: " << triMesh.texels.size();
}

void MikanTestApp::logTransform(const MikanTransform& xform)
{
	const MikanVector3f& s = xform.scale;
	const MikanVector3f& t = xform.position;
	const MikanQuatf& q = xform.rotation;

	MIKAN_LOG_INFO("TransformComponent") << "  Scale: " << s.x << ", " << s.y << ", " << s.z;
	MIKAN_LOG_INFO("TransformComponent") << "  Rotation: " << q.x << ", " << q.y << ", " << q.z << ", " << q.w;
	MIKAN_LOG_INFO("TransformComponent") << "  Position: " << t.x << ", " << t.y << ", " << t.z;
}