#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAPI.rfkh.h"
#endif

#include <memory>
#include <string>

using IMikanAPIPtr = std::shared_ptr<class IMikanAPI>;

// -- Render Target API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanRenderTargetAPI")) IMikanRenderTargetAPI
{
public:
	IMikanRenderTargetAPI() {}
	virtual ~IMikanRenderTargetAPI() {}

	METHOD()
	virtual MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) = 0;
	METHOD()
	virtual MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) = 0;
	METHOD()
	virtual MikanResponseFuture allocateRenderTargetTextures(const MikanRenderTargetDescriptor& descriptor) = 0;
	METHOD()
	virtual MikanResult writeColorRenderTargetTexture(void* apiColorTexturePtr) = 0;
	METHOD()
	virtual MikanResult writeDepthRenderTargetTexture(void* apiDepthTexturePtr, float zNear, float zFar) = 0;
	METHOD()
	virtual MikanResult publishRenderTargetTextures(MikanClientFrameRendered& frameInfo) = 0;
	METHOD()
	virtual MikanResponseFuture freeRenderTargetTextures() = 0;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanRenderTargetAPI_GENERATED
	#endif
};

// -- Script API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanScriptAPI")) IMikanScriptAPI
{
public:
	IMikanScriptAPI() {}
	virtual ~IMikanScriptAPI() {}

	METHOD()
	virtual MikanResponseFuture sendScriptMessage(const std::string& mesg) = 0; // returns MikanResult

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanScriptAPI_GENERATED
	#endif
};

// -- Spatial Anchor API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanSpatialAnchorAPI")) IMikanSpatialAnchorAPI
{
public:
	IMikanSpatialAnchorAPI() {}
	virtual ~IMikanSpatialAnchorAPI() {}

	METHOD()
	virtual MikanResponseFuture getSpatialAnchorList() = 0; // returns MikanSpatialAnchorList
	METHOD()
	virtual MikanResponseFuture getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) = 0; // returns MikanSpatialAnchorInfo
	METHOD()
	virtual MikanResponseFuture findSpatialAnchorInfoByName(const std::string& anchorName) = 0; // returns MikanSpatialAnchorInfo

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanSpatialAnchorAPI_GENERATED
	#endif
};

// -- Stencil API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanStencilAPI")) IMikanStencilAPI
{
public:
	IMikanStencilAPI() {}
	virtual ~IMikanStencilAPI() {}

	METHOD()
	virtual MikanResponseFuture getQuadStencilList() = 0; // returns MikanStencilList
	METHOD()
	virtual MikanResponseFuture getQuadStencil(MikanStencilID stencilId) = 0; // returns MikanStencilQuad
	METHOD()
	virtual MikanResponseFuture getBoxStencilList() = 0; // returns MikanStencilList
	METHOD()
	virtual MikanResponseFuture getBoxStencil(MikanStencilID stencilId) = 0; // returns MikanStencilBox
	METHOD()
	virtual MikanResponseFuture getModelStencilList() = 0; // returns MikanStencilList
	METHOD()
	virtual MikanResponseFuture getModelStencil(MikanStencilID stencilId) = 0; // returns MikanStencilModel
	METHOD()
	virtual MikanResponseFuture getModelStencilRenderGeometry(MikanStencilID stencilId) = 0; // returns MikanStencilModel

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanStencilAPI_GENERATED
	#endif
};

// -- Video Source API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanVideoSourceAPI")) IMikanVideoSourceAPI
{
public:
	IMikanVideoSourceAPI() {}
	virtual ~IMikanVideoSourceAPI() {}

	METHOD()
	virtual MikanResponseFuture getVideoSourceIntrinsics() = 0; // returns MikanVideoSourceIntrinsics
	METHOD()
	virtual MikanResponseFuture getVideoSourceMode() = 0; // returns MikanVideoSourceMode
	METHOD()
	virtual MikanResponseFuture getVideoSourceAttachment() = 0; // returns MikanVideoSourceAttachmentInfo

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanVideoSourceAPI_GENERATED
	#endif
};

// -- VRDevice API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanVRDeviceAPI")) IMikanVRDeviceAPI
{
public:
	IMikanVRDeviceAPI() {}
	virtual ~IMikanVRDeviceAPI() {}

	METHOD()
	virtual MikanResponseFuture getVRDeviceList() = 0; // returns MikanVRDeviceList
	METHOD()
	virtual MikanResponseFuture getVRDeviceInfo(MikanVRDeviceID deviceId) = 0; // returns MikanVRDeviceInfo
	METHOD()
	virtual MikanResponseFuture subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId) = 0; // returns MikanResponse
	METHOD()
	virtual MikanResponseFuture unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId) = 0; // returns MikanResponse

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanVRDeviceAPI_GENERATED
	#endif
};

// -- Mikan API -----
class MIKAN_API CLASS(Serialization::CodeGenModule("MikanAPI")) IMikanAPI
{
public:
	IMikanAPI() = default;
	virtual ~IMikanAPI() = default;

	// Initialize the Mikan API
	METHOD()
	static IMikanAPIPtr createMikanAPI();
	METHOD()
	virtual MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) = 0;
	METHOD()
	virtual bool getIsInitialized() = 0;

	// Sub API accessors
	METHOD()
	virtual int getCoreSDKVersion() const = 0;
	METHOD()
	virtual std::string getClientUniqueID() const = 0;
	METHOD()
	virtual IMikanRenderTargetAPI* getRenderTargetAPI() const = 0;
	METHOD()
	virtual IMikanVideoSourceAPI* getVideoSourceAPI() const = 0;
	METHOD()
	virtual IMikanVRDeviceAPI* getVRDeviceAPI() const = 0;
	METHOD()
	virtual IMikanScriptAPI* getScriptAPI() const = 0;
	METHOD()
	virtual IMikanStencilAPI* getStencilAPI() const = 0;
	METHOD()
	virtual IMikanSpatialAnchorAPI* getSpatialAnchorAPI() const = 0;

	// Set client properties before calling connect
	METHOD()
	virtual MikanResult setClientInfo(const MikanClientInfo& clientInfo) = 0;
	METHOD()
	virtual MikanResult connect(const std::string& host="", const std::string& port="") = 0;
	METHOD()
	virtual bool getIsConnected() = 0;
	METHOD()
	virtual MikanResult fetchNextEvent(MikanEventPtr& out_event) = 0;
	METHOD()
	virtual MikanResult disconnect() = 0;
	METHOD()
	virtual MikanResult shutdown() = 0;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	IMikanAPI_GENERATED
	#endif
};


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanAPI_GENERATED
#endif